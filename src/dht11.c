#include "dht11.h"
#include "utils.h"

int consume_bit(dht11_measurement_t *probe_data, int position, uint8_t *value) {
    int64_t diff = probe_data[position+1].us - probe_data[position].us;

    if (diff > 49) {
        *value = 1;
    }
    else {
        *value = 0;
    }

    return position + 2;
}

int consume_byte(dht11_measurement_t *probe_data, int position, uint8_t *value) {
    uint8_t bit;
    *value = 0;
    for (int i = 7; i >= 0; i--)
    {
        position = consume_bit(probe_data, position, &bit);
        *value |= bit << i;
    }
    return position;
}

int get_measurement(dht11_measurement_t *probe_data, dht11_data_t *empty_data)
{
    // skip non-data segments
    int position = 5;
    position = consume_byte(probe_data, position, &empty_data->humidity_integral);
    position = consume_byte(probe_data, position, &empty_data->humidity_decimal);
    position = consume_byte(probe_data, position, &empty_data->temperature_integral);
    position = consume_byte(probe_data, position, &empty_data->temperature_decimal);
    position = consume_byte(probe_data, position, &empty_data->crc);
    uint8_t all_sum = empty_data->humidity_integral + empty_data->humidity_decimal + empty_data->temperature_integral + empty_data->temperature_decimal;
    return ((all_sum > 0) && (all_sum == empty_data->crc)) ? (ESP_OK) : (MEASUREMENT_ERROR);
}
