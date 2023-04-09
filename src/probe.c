#include "probe.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "config.h"
#include "utils.h"

#define ESP_INTR_FLAG_DEFAULT (0)

static void IRAM_ATTR gpio_dht_isr_handler(void *arg)
{
    probe_t *dht = (probe_t*) arg;

    if (probe_get_captured_data_count(dht) >= DHT11_FRAME_SIZE)
    {
        return;
    }
    
    int level = gpio_get_level(GPIO_DHT11);

    if (probe_get_captured_data_count(dht) == 0 && level == 1)
    {
        // ignore glitch
        return;
    }

    probe_measurement_t data = {
        .level = level,
        .us = microseconds(dht->timer)
    };

    dht->buffer[dht->captured_data++] = data;
}

probe_t probe_init(gpio_num_t pin, uint16_t queue_size, probe_measurement_t *buffer)
{
    // install gpio isr service
    gptimer_handle_t timer = hw_timer_init();
    probe_t dht = {
        .pin = pin,
        .queue_size = queue_size,
        .timer = timer,
        .buffer = buffer,
        .captured_data = 0
    };
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
    return dht;
}

void probe_enable(probe_t *dht) 
{
    gpio_isr_handler_add(dht->pin, gpio_dht_isr_handler, dht);
}

probe_measurement_t* dht11_get_data(probe_t *dht)
{
    return dht->buffer;
}

int probe_has_data(probe_t *dht)
{
    return dht->captured_data == dht->queue_size;
}

void probe_reset_capture(probe_t *dht)
{
    hw_timer_reset(dht->timer);
    dht->captured_data = 0;
}

int probe_get_captured_data_count(probe_t *dht) 
{
    return dht->captured_data;
}