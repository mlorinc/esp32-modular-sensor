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

#define GPIO_DHT11 (14)
#define WAIT_TIMEOUT_ERROR (-1)
#define TRANSMIT_TIMEOUT_ERROR (2)
#define TRANSMIT_BYTE_TIMEOUT_ERROR (255)

#define PULL_UP_TIMEOUT (40)
#define DHT_ZERO_RESPONSE_TIMEOUT (80)
#define DHT_PULL_UP_RESPONSE_TIMEOUT (80)
#define TRANSMIT_START_TIMEOUT (50)
#define DATA_FRAME_START_TIMEOUT (70)
#define DATA_FRAME_END_TIMEOUT (70)

int64_t waitForLevel(int level, int64_t u_timeout)
{
    int64_t start = esp_timer_get_time();
    int64_t elapsed_time;
    while ((elapsed_time = esp_timer_get_time() - start) <= u_timeout)
    {
        int value = gpio_get_level(GPIO_DHT11);
        // printf("level: %d\n", value);
        if (value == level)
        {
            return u_timeout - elapsed_time;
        }
    }
    return WAIT_TIMEOUT_ERROR;
}

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

int64_t init_stream()
{
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_DHT11, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(GPIO_DHT11, 0));
    // vTaskDelay(20 / portTICK_PERIOD_MS);
    waitForLevel(1, 20000);
    ESP_ERROR_CHECK(gpio_set_level(GPIO_DHT11, 1));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_DHT11, GPIO_MODE_INPUT));
    int64_t pull_up_time = waitForLevel(1, PULL_UP_TIMEOUT);

    if (pull_up_time == WAIT_TIMEOUT_ERROR)
    {
        ESP_LOGE("waitForLevel", "init_stream pull_up_time timed out");
        return WAIT_TIMEOUT_ERROR;
    }

    int64_t dht_zero_response = waitForLevel(0, DHT_ZERO_RESPONSE_TIMEOUT + pull_up_time);
    if (dht_zero_response == WAIT_TIMEOUT_ERROR)
    {
        ESP_LOGE("waitForLevel", "init_stream dht_zero_response timed out");
        return WAIT_TIMEOUT_ERROR;
    }

    int64_t dht_pull_up_response = waitForLevel(1, DHT_PULL_UP_RESPONSE_TIMEOUT + dht_zero_response);

    if (dht_pull_up_response == WAIT_TIMEOUT_ERROR)
    {
        ESP_LOGE("waitForLevel", "init_stream dht_pull_up_response timed out");
        return WAIT_TIMEOUT_ERROR;
    }

    return dht_pull_up_response;
}

uint8_t transmit_bit(int64_t u_accepted_delay)
{
    int64_t init_timeout = waitForLevel(0, TRANSMIT_START_TIMEOUT + u_accepted_delay);

    if (init_timeout == WAIT_TIMEOUT_ERROR)
    {
        ESP_LOGE("waitForLevel", "transmit_bit init_timeout timed out");
        return TRANSMIT_TIMEOUT_ERROR;
    }

    int64_t data_start = waitForLevel(1, DATA_FRAME_START_TIMEOUT + init_timeout);

    if (data_start == WAIT_TIMEOUT_ERROR)
    {
        ESP_LOGE("waitForLevel", "transmit_bit data_start timed out");
        return TRANSMIT_TIMEOUT_ERROR;
    }

    int64_t data_duration = DATA_FRAME_START_TIMEOUT - waitForLevel(0, DATA_FRAME_END_TIMEOUT);

    // printf("duration: %lld\n", end);
    if (data_duration == WAIT_TIMEOUT_ERROR)
    {
        ESP_LOGE("waitForLevel", "transmit_bit data_duration timed out");
        return TRANSMIT_TIMEOUT_ERROR;
    }

    return (data_duration >= 31) ? (1) : (0);
}

uint8_t transmit_byte(int64_t u_accepted_delay)
{
    uint8_t value = transmit_bit(u_accepted_delay) << 7;

    if (value == TRANSMIT_BYTE_TIMEOUT_ERROR)
    {
        ESP_LOGE("waitForLevel", "transmit_byte value[7] timed out");
        return 255;
    }

    for (int i = 6; i >= 0; i--)
    {
        uint8_t bit = transmit_bit(0);
        if (bit == TRANSMIT_TIMEOUT_ERROR)
        {
            ESP_LOGE("waitForLevel", "transmit_byte value[%d] timed out", i);
            return TRANSMIT_BYTE_TIMEOUT_ERROR;
        }
        value |= bit << i;
    }
    return value;
}

void dht11_task(void *pvParameter)
{
    while (1)
    {   
        printf("Time: %s\n", get_utc_time());

        vTaskDelay(5000 / portTICK_PERIOD_MS);
        // portMUX_TYPE timeCriticalMutex = portMUX_INITIALIZER_UNLOCKED;
        // taskENTER_CRITICAL(&timeCriticalMutex);
        int64_t timeout = init_stream();
        if (timeout == WAIT_TIMEOUT_ERROR)
        {
            ESP_LOGE("transmit", "init_stream timed out");
            goto ERROR;
        }

        uint8_t integral_humidity = transmit_byte(timeout);

        if (integral_humidity == TRANSMIT_BYTE_TIMEOUT_ERROR)
        {
            ESP_LOGE("transmit", "integral_humidity timed out");
            goto ERROR;
        }

        uint8_t decimal_humidity = transmit_byte(0);

        if (decimal_humidity == TRANSMIT_BYTE_TIMEOUT_ERROR)
        {
            ESP_LOGE("transmit", "decimal_humidity timed out");
            goto ERROR;
        }
        uint8_t integral_temperature = transmit_byte(0);

        if (integral_temperature == TRANSMIT_BYTE_TIMEOUT_ERROR)
        {
            ESP_LOGE("transmit", "integral_temperature timed out");
            goto ERROR;
        }
        uint8_t decimal_temperature = transmit_byte(0);

        if (decimal_temperature == TRANSMIT_BYTE_TIMEOUT_ERROR)
        {
            ESP_LOGE("transmit", "decimal_temperature timed out");
            goto ERROR;
        }
        uint8_t crc = transmit_byte(0);

        if (crc == TRANSMIT_BYTE_TIMEOUT_ERROR)
        {
            ESP_LOGE("transmit", "crc timed out");
            goto ERROR;
        }

        // taskEXIT_CRITICAL(&timeCriticalMutex);
        uint8_t all_sum = integral_humidity + decimal_humidity + integral_temperature + decimal_temperature;
        ESP_LOGI("data", "CRC: %d\n", (all_sum > 0) && (all_sum == crc));
        ESP_LOGI("data", "temperature: %d.%d\n Celsius", integral_temperature, decimal_temperature);
        ESP_LOGI("data", "humidity: %d.%d%%\n", integral_humidity, decimal_humidity);
        continue;
        
        ERROR:
            // taskEXIT_CRITICAL(&timeCriticalMutex);
            continue;
    }
}

void app_main()
{   
    esp_rom_gpio_pad_select_gpio(GPIO_DHT11);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_DHT11, GPIO_MODE_INPUT));

    wifi_init();
    wifi_start();
    ntp_init();
    
    if (ntp_wait(30000000) != ESP_OK)
    {
        while (1)
        {
            /* noop */
        }
    }
    wifi_stop();
    
    xTaskCreate(&dht11_task, "dht11_task", 2048, NULL, 5, NULL);
}