#ifndef __H_UTILS
#define __H_UTILS
#include "driver/gptimer.h"
#define WAIT_TIMEOUT_ERROR (-1)

uint64_t microseconds(gptimer_handle_t timer);
gptimer_handle_t hw_timer_init();
int hw_timer_pause(gptimer_handle_t timer);
int hw_timer_reset(gptimer_handle_t timer);
int hw_timer_stop(gptimer_handle_t timer);
float decimal_to_float(uint8_t number);
#endif