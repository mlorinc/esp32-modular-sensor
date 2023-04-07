#include <time.h>
#include "utc.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_log.h"
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

void ntp_stop(void) {
    sntp_stop();
}

uint8_t is_synced() {
    return synced;
}

void ntp_sync_callback(struct timeval *t) {
    printf("%s\n", get_utc_time());
    sntp_sync_status_t status = sntp_get_sync_status();
    synced = (status == SNTP_SYNC_STATUS_COMPLETED) ? (1) : (0);
    

    // if (time_info.tm_year == 70)
    // {
    //     ESP_LOGI("ntp", "sync year was not correct: year=%d", time_info.tm_year);
    //     synced = 0;
    // }
    // else if (time_info.tm_year != 70 && status == SNTP_SYNC_STATUS_COMPLETED){
    //     ESP_LOGI("ntp", "sync is OK");
    //     synced = 1;
    // }
    // else {
    //     ESP_LOGE("ntp", "unknown state: year=%d, status=%d", time_info.tm_year, status);
    // }
}

void ntp_init(void) {
    ESP_LOGI("ntp", "ntp init");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(ntp_sync_callback);
    sntp_init();
    ESP_LOGI("ntp", "ntp started");
}
