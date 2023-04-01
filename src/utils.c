#include "utils.h"
#include "esp_timer.h"

int64_t wait_for_level(gpio_num_t pin, int level, int64_t u_timeout)
{
    int64_t start = esp_timer_get_time();
    int64_t elapsed_time;
    while ((elapsed_time = esp_timer_get_time() - start) <= u_timeout)
    {
        int value = gpio_get_level(pin);
        // printf("level: %d\n", value);
        if (value == level)
        {
            return u_timeout - elapsed_time;
        }
    }
    return WAIT_TIMEOUT_ERROR;
}