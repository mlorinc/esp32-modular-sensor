#ifndef __H_PROBE
#define __H_PROBE
#define DHT11_FRAME_SIZE (86)

#include "driver/gpio.h"

typedef struct dht11_measurement {
    uint8_t level;
    uint64_t us;
} dht11_measurement_t;

void register_pin(gpio_num_t pin, int queue_size);
dht11_measurement_t* get_pin_data(gpio_num_t pin);
int is_probe_full(gpio_num_t pin);
int get_probe_size();
void reset_probe();

#endif