#include "dht11.h"
#include "utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

static int64_t init_stream(gpio_num_t pin)
{
    ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(pin, 0));
    // wait_for_level(pin, 1, 20000);
    ets_delay_us(20000);
    ESP_ERROR_CHECK(gpio_set_level(pin, 1));
    ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_INPUT));
    int64_t pull_up_time = wait_for_level(pin, 1, PULL_UP_TIMEOUT);

    if (pull_up_time == WAIT_TIMEOUT_ERROR)
    {
        // ESP_LOGE("wait_for_level", "init_stream pull_up_time timed out");
        return WAIT_TIMEOUT_ERROR;
    }

    int64_t dht_zero_response = wait_for_level(pin, 0, DHT_ZERO_RESPONSE_TIMEOUT + pull_up_time);
    if (dht_zero_response == WAIT_TIMEOUT_ERROR)
    {
        // ESP_LOGE("wait_for_level", "init_stream dht_zero_response timed out");
        return WAIT_TIMEOUT_ERROR;
    }

    int64_t dht_pull_up_response = wait_for_level(pin, 1, DHT_PULL_UP_RESPONSE_TIMEOUT + dht_zero_response);

    if (dht_pull_up_response == WAIT_TIMEOUT_ERROR)
    {
        // ESP_LOGE("wait_for_level", "init_stream dht_pull_up_response timed out");
        return WAIT_TIMEOUT_ERROR;
    }

    return dht_pull_up_response;
}

static uint8_t transmit_bit(gpio_num_t pin, int64_t u_accepted_delay)
{
    int64_t init_timeout = wait_for_level(pin, 0, TRANSMIT_START_TIMEOUT + u_accepted_delay);

    if (init_timeout == WAIT_TIMEOUT_ERROR)
    {
        // ESP_LOGE("wait_for_level", "transmit_bit init_timeout timed out");
        return TRANSMIT_TIMEOUT_ERROR;
    }

    int64_t data_start = wait_for_level(pin, 1, DATA_FRAME_START_TIMEOUT + init_timeout);

    if (data_start == WAIT_TIMEOUT_ERROR)
    {
        // ESP_LOGE("wait_for_level", "transmit_bit data_start timed out");
        return TRANSMIT_TIMEOUT_ERROR;
    }

    int64_t data_duration = DATA_FRAME_START_TIMEOUT - wait_for_level(pin, 0, DATA_FRAME_END_TIMEOUT);

    if (data_duration == WAIT_TIMEOUT_ERROR)
    {
        // ESP_LOGE("wait_for_level", "transmit_bit data_duration timed out");
        return TRANSMIT_TIMEOUT_ERROR;
    }

    return (data_duration >= 31) ? (1) : (0);
}

static uint8_t transmit_byte(gpio_num_t pin, int64_t u_accepted_delay)
{
    uint8_t value = transmit_bit(pin, u_accepted_delay) << 7;

    if (value == TRANSMIT_BYTE_TIMEOUT_ERROR)
    {
        // ESP_LOGE("wait_for_level", "transmit_byte value[7] timed out");
        return 255;
    }

    for (int i = 6; i >= 0; i--)
    {
        uint8_t bit = transmit_bit(pin, 0);
        if (bit == TRANSMIT_TIMEOUT_ERROR)
        {
            // ESP_LOGE("wait_for_level", "transmit_byte value[%d] timed out", i);
            return TRANSMIT_BYTE_TIMEOUT_ERROR;
        }
        value |= bit << i;
    }
    return value;
}

int get_measurement(gpio_num_t pin, dht11_data_t *empty_data)
{
    portMUX_TYPE timeCriticalMutex = portMUX_INITIALIZER_UNLOCKED;
    taskENTER_CRITICAL(&timeCriticalMutex);
    int64_t timeout = init_stream(pin);
    char *err;
    if (timeout == WAIT_TIMEOUT_ERROR)
    {
        err = "init_stream timed out";
        // ESP_LOGE("transmit", "init_stream timed out");
        goto ERROR;
    }

    uint8_t integral_humidity = transmit_byte(pin, timeout);

    if (integral_humidity == TRANSMIT_BYTE_TIMEOUT_ERROR)
    {
        err = "integral_humidity timed out";
        // ESP_LOGE("transmit", "integral_humidity timed out");
        goto ERROR;
    }

    uint8_t decimal_humidity = transmit_byte(pin, 0);

    if (decimal_humidity == TRANSMIT_BYTE_TIMEOUT_ERROR)
    {
        err = "decimal_humidity timed out";
        // ESP_LOGE("transmit", "decimal_humidity timed out");
        goto ERROR;
    }
    uint8_t integral_temperature = transmit_byte(pin, 0);

    if (integral_temperature == TRANSMIT_BYTE_TIMEOUT_ERROR)
    {
        err = "integral_temperature timed out";
        // ESP_LOGE("transmit", "integral_temperature timed out");
        goto ERROR;
    }
    uint8_t decimal_temperature = transmit_byte(pin, 0);

    if (decimal_temperature == TRANSMIT_BYTE_TIMEOUT_ERROR)
    {
        err = "decimal_temperature timed out";
        // ESP_LOGE("transmit", "decimal_temperature timed out");
        goto ERROR;
    }
    uint8_t crc = transmit_byte(pin, 0);

    if (crc == TRANSMIT_BYTE_TIMEOUT_ERROR)
    {
        err = "crc timed out";
        // ESP_LOGE("transmit", "crc timed out");
        goto ERROR;
    }

    taskEXIT_CRITICAL(&timeCriticalMutex);
    uint8_t all_sum = integral_humidity + decimal_humidity + integral_temperature + decimal_temperature;
    empty_data->crc = crc;
    empty_data->humidity_decimal = decimal_humidity;
    empty_data->humidity_integral = integral_humidity;
    empty_data->temperature_decimal = decimal_temperature;
    empty_data->temperature_integral = integral_temperature;

    return ((all_sum > 0) && (all_sum == crc)) ? (ESP_OK) : (MEASUREMENT_ERROR);

ERROR:
    taskEXIT_CRITICAL(&timeCriticalMutex);
    ESP_LOGE("transmit", "%s\n", err);
    return TRANSMIT_BYTE_TIMEOUT_ERROR;
}
