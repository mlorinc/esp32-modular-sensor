#ifndef __H_UTC
#define __H_UTC
#include <stdio.h>

#define TIME_BUFFER_SIZE (64)

/**
 * Get time in UTC format and +0 timezone.
*/
char *get_utc_time(void);
/**
 * Init ntp client.
*/
void ntp_init(void);
/**
 * Stop ntp client.
*/
void ntp_stop(void);

/**
 * Check if time is set.
*/
uint8_t is_synced();
#endif