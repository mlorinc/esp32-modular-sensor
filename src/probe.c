#include "probe.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "config.h"
#include "utils.h"

#define ESP_INTR_FLAG_DEFAULT (0)

dht11_measurement_t dht_data[DHT11_FRAME_SIZE] = { 0 };
int size = 0;
gptimer_handle_t timer;

static void IRAM_ATTR gpio_dht_isr_handler(void *arg)
{
    if (size >= DHT11_FRAME_SIZE)
    {
        return;
    }
    
    int level = gpio_get_level(GPIO_DHT11);

    if (size == 0 && level == 1)
    {
        // ignore glitch
        return;
    }

    dht11_measurement_t data = {
        .level = level,
        .us = microseconds(timer)
    };

    dht_data[size++] = data;
}

void register_pin(gpio_num_t pin, int queue_size)
{
    // install gpio isr service
    timer = hw_timer_init();
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
    gpio_isr_handler_add(pin, gpio_dht_isr_handler, NULL);
}

dht11_measurement_t *get_pin_data(gpio_num_t pin)
{
    return dht_data;
}

int is_probe_full(gpio_num_t pin)
{
    return size == DHT11_FRAME_SIZE;
}

void reset_probe() {
    hw_timer_reset(timer);
    size = 0;
}

int get_probe_size() {
    return size;
}