#ifndef __H_DHT11
#define __H_DHT11
#include <stdlib.h>
#include "driver/gpio.h"


#define TRANSMIT_TIMEOUT_ERROR (2)
#define TRANSMIT_BYTE_TIMEOUT_ERROR (255)
#define MEASUREMENT_ERROR (1)

#define PULL_UP_TIMEOUT (40)
#define DHT_ZERO_RESPONSE_TIMEOUT (80)
#define DHT_PULL_UP_RESPONSE_TIMEOUT (80)
#define TRANSMIT_START_TIMEOUT (50)
#define DATA_FRAME_START_TIMEOUT (70)
#define DATA_FRAME_END_TIMEOUT (70)

typedef struct dht11_data {
    uint8_t crc;
    uint8_t temperature_integral;
    uint8_t temperature_decimal;
    uint8_t humidity_integral;
    uint8_t humidity_decimal;
} dht11_data_t;

int get_measurement(gpio_num_t pin, dht11_data_t *empty_data);
#endif