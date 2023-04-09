#ifndef __H_MQTT
#define __H_MQTT

#include "dht11.h"

/**
 * Init MQTT client and register event loop.
 */
int mqtt_init();

/**
 * Send data to MQTT server.
 * @param data - data to be sent
 * @param length - length of data
 * @return message id if successful, -1 otherwise
 */
int mqtt_send_data(const char *data, int length);

/**
 * Check whether MQTT is connected.
 */
uint8_t mqtt_connected();

#endif