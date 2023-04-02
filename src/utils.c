#include "utils.h"
#include "esp_system.h"

/*
 * A simple helper function to print the raw timer counter value
 * and the counter value converted to seconds
 */
uint64_t microseconds(gptimer_handle_t timer) {
    uint64_t t;
    ESP_ERROR_CHECK(gptimer_get_raw_count(timer, &t));
    return t;
}

gptimer_handle_t hw_timer_init()
{
    gptimer_handle_t gptimer = NULL;
    /* Select and initialize basic parameters of the timer */
    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
    }; // default clock source is APB
    gptimer_new_timer(&config, &gptimer);

    // gptimer_alarm_config_t alarm = {
    //     .alarm_count = u_timeout,
    // };
    
    // gptimer_set_alarm_action(gptimer, &alarm);
    // gptimer_register_event_callbacks(gptimer, timer_interrupt, NULL);
    gptimer_enable(gptimer);
    gptimer_set_raw_count(gptimer, 0);
    gptimer_start(gptimer);
    return gptimer;
}

int hw_timer_pause(gptimer_handle_t timer) {
    return gptimer_stop(timer);
}

int hw_timer_reset(gptimer_handle_t timer) {
    return gptimer_set_raw_count(timer, 0);
}

int hw_timer_stop(gptimer_handle_t timer) {
    gptimer_stop(timer);
    gptimer_disable(timer);
    return gptimer_del_timer(timer);
}
