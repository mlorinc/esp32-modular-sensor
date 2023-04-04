#ifndef __H_UTC
#define __H_UTC
#include <stdio.h>

#define TIME_BUFFER_SIZE (64)

char *get_utc_time(void);
void ntp_init(void);
void ntp_stop(void);
int ntp_sync();
uint8_t is_synced();
#endif