#include <string.h>
#include <inttypes.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "wifi.h"
#include "utc.h"
#include "utils.h"
#include "dht11.h"
#include "config.h"
#include "probe.h"
#include "mqtt.h"
#include "temperature_predictor.h"
#include "array_queue.h"

#define MQTT_BUFFER_SIZE (256)
#define REFRESH_INTERVAL (0.5 * 1000 * 60) // 30s
#define STABILIZATION_WAIT (250)
#define FAILURE_WAIT (250)

float temperatures[8] = {0};
float humidities[8] = {0};
uint8_t measured_values = 0;

void report_measurement(char *buffer, dht11_data_t *data)
{
    ESP_LOGI("data", "CRC: %d\n", data->crc);
    ESP_LOGI("data", "temperature: %d.%d\n Celsius", data->temperature_integral, data->temperature_decimal);
    ESP_LOGI("data", "humidity: %d.%d%%\n", data->humidity_integral, data->humidity_decimal);
    memset(buffer, 0, MQTT_BUFFER_SIZE);
    sprintf(buffer, "{utc:\"%s\",temperature:%d.%d,humidity:%d.%d}", get_utc_time(), data->temperature_integral, data->temperature_decimal, data->humidity_integral, data->humidity_decimal);
    int length = strlen(buffer);
    ESP_LOGI("data", "mqtt: %s", buffer);
    if (mqtt_send_data(buffer, length) == -1)
    {
        ESP_LOGE("data", "could not send data through MQTT");
    }
}

void dht11_task(void *pvParameter)
{
    probe_measurement_t dht_data[DHT11_FRAME_SIZE] = {0};
    probe_t dht = probe_init(GPIO_DHT11, DHT11_FRAME_SIZE, dht_data);
    probe_enable(&dht);
    dht11_data_t data = {0};
    char buffer[MQTT_BUFFER_SIZE] = {0};

    while (!is_synced())
    {
        ESP_LOGE("ntp", "waiting ntp to be synced");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    ESP_LOGE("mqtt", "mqtt init");
    if (mqtt_init() != ESP_OK)
    {
        ESP_LOGE("mqtt", "mqtt not connected");
        return;
    }

    while (!mqtt_connected())
    {
        ESP_LOGE("mqtt", "waiting mqtt to be connected");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    ESP_LOGE("mqtt", "mqtt started");

    while (1)
    {
        if (!is_synced() || !mqtt_connected())
        {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI("wait", "Time: %s\n", get_utc_time());
        probe_reset_capture(&dht);
        dht11_begin_transmission(&dht);
        vTaskDelay(STABILIZATION_WAIT / portTICK_PERIOD_MS);

        if (!probe_has_data(&dht))
        {
            ESP_LOGI("wait", "not full abort, size: %d", probe_get_captured_data_count(&dht));
            vTaskDelay(FAILURE_WAIT / portTICK_PERIOD_MS);
            continue;
        }

        if (dht11_get_measurement(dht11_get_data(&dht), &data) == ESP_OK)
        {
            report_measurement(buffer, &data);
#ifdef PREDICTION_MODE
            queue_push(temperatures, 8, data.temperature_integral + decimal_to_float(data.temperature_decimal));
            queue_push(humidities, 8, data.humidity_integral + decimal_to_float(data.humidity_decimal));
            if (measured_values < 8)
            {
                measured_values++;
            }

            if (measured_values == 8)
            {
                float predicted_temperature = temperature_model_interfere(temperatures, humidities);
                ESP_LOGI("measurement", "predicted temperature: %f\n", predicted_temperature);
            }
            else if (measured_values > 8)
            {
                ESP_LOGE("measurement", "unexpected measured values state");
            }
            else
            {
                ESP_LOGI("measurement", "skipping prediction; not enough data %u/8", measured_values);
            }
#endif
        }
        else
        {
            ESP_LOGI("data", "error while measuring data from DHT11");
            vTaskDelay(FAILURE_WAIT / portTICK_PERIOD_MS);
            continue;
        }

        vTaskDelay((REFRESH_INTERVAL - STABILIZATION_WAIT) / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    esp_rom_gpio_pad_select_gpio(GPIO_DHT11);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_DHT11, GPIO_MODE_INPUT));

    if (temperature_model_init() != 0)
    {
        ESP_LOGE("tf", "could not init Tensorflow Lite");
        return;
    }

    if (wifi_init() != ESP_OK)
    {
        ESP_LOGE("wifi", "could not connect to the wifi");
        return;
    }

    ntp_init();
    xTaskCreate(&dht11_task, "dht11_task", 4096, NULL, 5, NULL);
}