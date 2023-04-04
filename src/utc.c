#include <time.h>
#include "utc.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "utils.h"

char time_string[TIME_BUFFER_SIZE] = { 0 };
uint8_t synced = 0;

char *get_utc_time(void) {
    time_t now;
    struct tm time_info;

    time(&now);
    
    localtime_r(&now, &time_info);
    memset(time_string, 0, TIME_BUFFER_SIZE);
    strftime(time_string, TIME_BUFFER_SIZE, "%Y-%m-%dT%H:%M:%SZ", &time_info);
    return time_string;
}

void time_sync_notification_cb(struct timeval *tv) {
    synced = 1;
}


void ntp_init(void) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    struct timeval t;
    sntp_sync_time(&t);
}

void ntp_stop(void) {
    sntp_stop();
}

uint8_t is_synced() {
    return synced;
}

int ntp_sync() {
    // sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    return ESP_OK;
}
