#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "wifi.h"
#include "utc.h"
#include "dht11.h"
#include "config.h"
#include "probe.h"
#include "rom/ets_sys.h"
#include <inttypes.h>

void stabilize(int level, int64_t duration)
{
    int64_t start = esp_timer_get_time();
    while (esp_timer_get_time() - start <= duration)
    {
        if (gpio_get_level(GPIO_DHT11) != 1)
        {
            start = esp_timer_get_time();
        }
    }
}

void dht11_task(void *pvParameter)
{
    dht11_data_t data = { 0 };
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    while (1)
    {
        printf("Time: %s\n", get_utc_time());
        reset_probe();
        gpio_set_direction(GPIO_DHT11, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_DHT11, 0);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_DHT11, 1);
        gpio_set_direction(GPIO_DHT11, GPIO_MODE_INPUT);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        if (!is_probe_full(GPIO_DHT11)) {
            ESP_LOGI("wait", "not full abort, size: %d", get_probe_size());
            vTaskDelay(250 / portTICK_PERIOD_MS);
            continue;
        }
        
        if (get_measurement(get_pin_data(GPIO_DHT11), &data) == ESP_OK) {
            ESP_LOGI("data", "CRC: %d\n", data.crc);
            ESP_LOGI("data", "temperature: %d.%d\n Celsius", data.temperature_integral, data.temperature_decimal);
            ESP_LOGI("data", "humidity: %d.%d%%\n", data.humidity_integral, data.humidity_decimal);
        }
        else {
            ESP_LOGI("data", "error while measuring data from DHT11");
        }

        vTaskDelay(4900 / portTICK_PERIOD_MS);

        // printf("Sequence: ");
        // dht11_measurement_t *data = get_pin_data(GPIO_DHT11);

        // for (size_t i = 0; i < DHT11_FRAME_SIZE; i++)
        // {
        //     printf("t=%" PRIu64 ";v=%d , ", data[i].us, data[i].level);
        // }
        // printf("\n");
    }
}

void app_main()
{
    esp_rom_gpio_pad_select_gpio(GPIO_DHT11);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_DHT11, GPIO_MODE_INPUT));

    // wifi_init();

    if (wifi_init() != ESP_OK)
    {
        ESP_LOGE("wifi", "could not connect to the wifi");
        return;
    }

    ntp_init();

    if (ntp_wait(30000000) != ESP_OK)
    {
        return;
    }
    // ntp_stop();
    // wifi_stop();
    // dht11_task(NULL);

    register_pin(GPIO_DHT11, 16);
    xTaskCreate(&dht11_task, "dht11_task", 2048, NULL, 5, NULL);
}