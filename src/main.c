#include <stdio.h>
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
#include "dht11.h"
#include "config.h"
#include "probe.h"
#include "mqtt.h"
#include "temperature_predictor.h"

#define MQTT_BUFFER_SIZE (256)
#define REFRESH_INTERVAL (0.5*1000 * 60) // 1 minutes
#define STABILIZATION_WAIT (250)
#define FAILURE_WAIT (250)

void report_measurement(char *buffer, dht11_data_t *data) {
    ESP_LOGI("data", "CRC: %d\n", data->crc);
    ESP_LOGI("data", "temperature: %d.%d\n Celsius", data->temperature_integral, data->temperature_decimal);
    ESP_LOGI("data", "humidity: %d.%d%%\n", data->humidity_integral, data->humidity_decimal);
    memset(buffer, 0, MQTT_BUFFER_SIZE);
    sprintf(buffer, "{utc:\"%s\",temperature:%d.%d,humidity:%d.%d}", get_utc_time(), data->temperature_integral, data->temperature_decimal, data->humidity_integral, data->humidity_decimal);
    int length = strlen(buffer);
    ESP_LOGI("data", "mqtt: %s", buffer);
    if (mqtt_send_data(buffer, length) == -1) {
        ESP_LOGE("data", "could not send data through MQTT");
    }
}

void dht11_task(void *pvParameter)
{    
    dht11_data_t data = { 0 };
    char buffer[MQTT_BUFFER_SIZE] = { 0 };

    while (!is_synced()) {
        ESP_LOGE("ntp", "waiting ntp to be synced");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);


    ESP_LOGE("mqtt", "mqtt init");
    if (mqtt_init() != ESP_OK) {
        ESP_LOGE("mqtt", "mqtt not connected");
        return;
    }

    while (!mqtt_connected()) {
        ESP_LOGE("mqtt", "waiting mqtt to be connected");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    ESP_LOGE("mqtt", "mqtt started");


    while (1)
    {
        if (!is_synced() || !mqtt_connected()) {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        printf("Time: %s\n", get_utc_time());
        reset_probe();
        gpio_set_direction(GPIO_DHT11, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_DHT11, 0);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_DHT11, 1);
        gpio_set_direction(GPIO_DHT11, GPIO_MODE_INPUT);
        vTaskDelay(STABILIZATION_WAIT / portTICK_PERIOD_MS);

        if (!is_probe_full(GPIO_DHT11)) {
            ESP_LOGI("wait", "not full abort, size: %d", get_probe_size());
            vTaskDelay(FAILURE_WAIT / portTICK_PERIOD_MS);
            continue;
        }
        
        if (get_measurement(get_pin_data(GPIO_DHT11), &data) == ESP_OK) {
            report_measurement(buffer, &data);
        }
        else {
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

    if (wifi_init() != ESP_OK)
    {
        ESP_LOGE("wifi", "could not connect to the wifi");
        return;
    }
    
    ntp_init();
    register_pin(GPIO_DHT11, 16);
    xTaskCreate(&dht11_task, "dht11_task", 4096, NULL, 5, NULL);
}