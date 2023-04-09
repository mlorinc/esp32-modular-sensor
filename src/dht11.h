#ifndef __H_DHT11
#define __H_DHT11
#include "driver/gpio.h"
#include "probe.h"

#define TRANSMIT_TIMEOUT_ERROR (2)
#define TRANSMIT_BYTE_TIMEOUT_ERROR (255)
#define MEASUREMENT_ERROR (1)

#define PULL_UP_TIMEOUT (40)
#define DHT_ZERO_RESPONSE_TIMEOUT (80)
#define DHT_PULL_UP_RESPONSE_TIMEOUT (80)
#define TRANSMIT_START_TIMEOUT (50)
#define DATA_FRAME_START_TIMEOUT (70)
#define DATA_FRAME_END_TIMEOUT (70)

/**
 * DHT11 data gathered during probing.
 */
typedef struct dht11_data
{
    uint8_t crc;
    uint8_t temperature_integral;
    uint8_t temperature_decimal;
    uint8_t humidity_integral;
    uint8_t humidity_decimal;
} dht11_data_t;

/**
 * Analyze captured data over time and return filled dht11_data struct.
 * @param probe_data measured data over time
 * @param empty_data reference to created empty dht11_data
 * @return ESP_OK if crc is correct, MEASUREMENT_ERROR otherwise
 */
int dht11_get_measurement(probe_measurement_t *probe_data, dht11_data_t *empty_data);
void dht11_begin_transmission(probe_t *dht);
#endif