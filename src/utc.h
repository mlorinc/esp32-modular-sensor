#ifndef __H_UTC
#define __H_UTC
#include <stdio.h>

extern char time_string[64];

char *get_utc_time(void);
void ntp_init(void);
void ntp_stop(void);
int ntp_wait(int64_t timeout);
#endif