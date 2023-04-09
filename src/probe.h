#ifndef __H_PROBE
#define __H_PROBE
#define DHT11_FRAME_SIZE (86)

#include "driver/gpio.h"
#include "driver/gptimer.h"

typedef struct probe_measurement
{
    uint8_t level;
    uint64_t us;
} probe_measurement_t;

typedef struct probe
{
    gpio_num_t pin;
    uint16_t queue_size;
    gptimer_handle_t timer;
    probe_measurement_t *buffer;
    int captured_data;
} probe_t;

/**
 * Create new probe object to watch voltage edge changes.
 * @param pin - sensor pin
 * @param queue_size - number of expected data
 * @param buffer - data storage
 * @returns new probe object
 */
probe_t probe_init(gpio_num_t pin, uint16_t queue_size, probe_measurement_t *buffer);

/**
 * Enable interrupt on probe object.
 */
void probe_enable(probe_t *dht);

/**
 * Get capture data.
 */
probe_measurement_t *dht11_get_data(probe_t *dht);

/**
 * Check whether required data has been capture.
 */
int probe_has_data(probe_t *dht);

/**
 * Get number of data captured.
 */
int probe_get_captured_data_count(probe_t *dht);

/**
 * Reset probe to initial state.
 */
void probe_reset_capture(probe_t *dht);

#endif