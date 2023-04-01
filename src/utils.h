#ifndef __H_UTILS
#define __H_UTILS
#include <stdlib.h>
#include "driver/gpio.h"
#define WAIT_TIMEOUT_ERROR (-1)

int64_t wait_for_level(gpio_num_t pin, int level, int64_t u_timeout);
#endif