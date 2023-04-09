#include "dht11.h"
#include "utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void dht11_begin_transmission(probe_t *dht)
{
    gpio_set_direction(dht->pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht->pin, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gpio_set_level(dht->pin, 1);
    gpio_set_direction(dht->pin, GPIO_MODE_INPUT);
}

int dht11_consume_bit(probe_measurement_t *probe_data, int position, uint8_t *value) {
    int64_t diff = probe_data[position+1].us - probe_data[position].us;

    if (diff > 49) {
        *value = 1;
    }
    else {
        *value = 0;
    }

    return position + 2;
}

int dht11_consume_byte(probe_measurement_t *probe_data, int position, uint8_t *value) {
    uint8_t bit;
    *value = 0;
    for (int i = 7; i >= 0; i--)
    {
        position = dht11_consume_bit(probe_data, position, &bit);
        *value |= bit << i;
    }
    return position;
}

int dht11_get_measurement(probe_measurement_t *probe_data, dht11_data_t *empty_data)
{
    // skip non-data segments
    int position = 5;
    position = dht11_consume_byte(probe_data, position, &empty_data->humidity_integral);
    position = dht11_consume_byte(probe_data, position, &empty_data->humidity_decimal);
    position = dht11_consume_byte(probe_data, position, &empty_data->temperature_integral);
    position = dht11_consume_byte(probe_data, position, &empty_data->temperature_decimal);
    position = dht11_consume_byte(probe_data, position, &empty_data->crc);
    uint8_t all_sum = empty_data->humidity_integral + empty_data->humidity_decimal + empty_data->temperature_integral + empty_data->temperature_decimal;
    return ((all_sum > 0) && (all_sum == empty_data->crc)) ? (ESP_OK) : (MEASUREMENT_ERROR);
}
