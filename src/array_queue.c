#include "array_queue.h"

void queue_push(float *data, uint8_t size, float value) {
    for (uint8_t i = size - 1; i >= 1; i--)
    {
        data[i-1] = data[i];
    }
    data[size-1] = value;
}