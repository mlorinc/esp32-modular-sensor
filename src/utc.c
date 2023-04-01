#include "utc.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include <time.h>

char time_string[64] = { 0 };

char *get_utc_time(void) {
    time_t now;
    struct tm time_info;

    time(&now);

    localtime_r(&now, &time_info);
    strftime(time_string, sizeof(time_string), "%c", &time_info);
    return time_string;
}

void ntp_init(void) {
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void ntp_stop(void) {
    sntp_stop();
}

int ntp_wait(int64_t u_timeout) {
    int64_t start = esp_timer_get_time();
    int64_t elapsed_time;
    while ((elapsed_time = esp_timer_get_time() - start) <= u_timeout)
    {
        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
        {
            return ESP_OK;
        }
    }
    return ~ESP_OK;
}