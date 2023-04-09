#ifndef __H_UTILS
#define __H_UTILS
#include "driver/gptimer.h"
#define WAIT_TIMEOUT_ERROR (-1)

/**
 * Get microseconds from timer.
 */
uint64_t microseconds(gptimer_handle_t timer);

/**
 * Initialize general purpose timer to measure microseconds.
 */
gptimer_handle_t hw_timer_init();

/**
 * Pause timer.
 */
int hw_timer_pause(gptimer_handle_t timer);
/**
 * Reset timer to 0.
 */
int hw_timer_reset(gptimer_handle_t timer);
/**
 * Clear and deinit timer.
 */
int hw_timer_stop(gptimer_handle_t timer);
/**
 * Convert integral and decimal part to one float.
 */
float decimal_to_float(uint8_t number);
#endif