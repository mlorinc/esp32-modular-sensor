#ifndef __H_UTILS
#define __H_UTILS
#include <stdlib.h>
#include "driver/gptimer.h"
#define WAIT_TIMEOUT_ERROR (-1)

uint64_t microseconds(gptimer_handle_t timer);
gptimer_handle_t hw_timer_init();
int hw_timer_pause(gptimer_handle_t timer);
int hw_timer_reset(gptimer_handle_t timer);
int hw_timer_stop(gptimer_handle_t timer);

#endif