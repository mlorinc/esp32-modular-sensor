#ifndef __H_MQTT
#define __H_MQTT

#include "dht11.h"

int mqtt_init();
int mqtt_send_data(const char* data, int length);
uint8_t mqtt_connected();

#endif