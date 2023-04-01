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

#define GPIO_DHT11 (14)

void stabilize(int level, int64_t duration) {
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
    dht11_data_t data;
    while (1)
    {   
        printf("Time: %s\n", get_utc_time());

        vTaskDelay(5000 / portTICK_PERIOD_MS);
        if (get_measurement(GPIO_DHT11, &data) == ESP_OK) {
            ESP_LOGI("data", "CRC: %d\n", data.crc);
            ESP_LOGI("data", "temperature: %d.%d\n Celsius", data.temperature_integral, data.temperature_decimal);
            ESP_LOGI("data", "humidity: %d.%d%%\n", data.humidity_integral, data.humidity_decimal);
        }
        else {
            ESP_LOGI("data", "error while measuring data from DHT11");
        }
    }
}

void app_main()
{   
    esp_rom_gpio_pad_select_gpio(GPIO_DHT11);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_DHT11, GPIO_MODE_INPUT));

    // wifi_init();
    
    // if (wifi_start() != ESP_OK) {
    //     ESP_LOGE("wifi", "could not connect to the wifi");
    //     return;
    // }

    // ntp_init();
    
    // if (ntp_wait(30000000) != ESP_OK)
    // {
    //     return;
    // }
    // ntp_stop();
    // wifi_stop();

    xTaskCreate(&dht11_task, "dht11_task", 2048, NULL, 5, NULL);
}