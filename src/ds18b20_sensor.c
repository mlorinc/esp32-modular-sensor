// Taken from the first TOI lab

#include "ds18b20_sensor.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

#define GPIO_DS18B20_0 (2)
#define MAX_DEVICES (4)
#define DS18B20_RESOLUTION (DS18B20_RESOLUTION_12_BIT)

OneWireBus *owb;
DS18B20_Info *ds18b20_info = NULL;

void ds18b20_sensor_init()
{
    int num_devices = 0;
    // Create a 1-Wire bus, using the RMT timeslot driver
    owb_rmt_driver_info rmt_driver_info;
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0,
                             RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true); // enable CRC check for ROM code

    // Find all connected devices
    ESP_LOGI("one-wire", "Find devices:\n");
    OneWireBus_ROMCode device_rom_codes[MAX_DEVICES] = {0};
    OneWireBus_SearchState search_state = {0};
    bool found = false;
    owb_search_first(owb, &search_state, &found);
    while (found)
    {
        char rom_code_s[17];
        owb_string_from_rom_code(search_state.rom_code, rom_code_s,
                                 sizeof(rom_code_s));
        printf("  %d : %s\n", num_devices, rom_code_s);
        device_rom_codes[num_devices] = search_state.rom_code;
        ++num_devices;
        owb_search_next(owb, &search_state, &found);
    }
    ESP_LOGI("one-wire", "Found %d device%s", num_devices,
             num_devices == 1 ? "" : "s");

    if (num_devices == 1) {
        ds18b20_info = ds18b20_malloc();
        ESP_LOGI("one-wire", "Single device optimisations enabled");
        ds18b20_init_solo(ds18b20_info, owb);
        ds18b20_use_crc(ds18b20_info, true);
        // enable CRC check on all reads
        ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
    }
    else {
        return;
    }
}

float ds18b20_sensor_get()
{
    if (ds18b20_info == NULL) {
        return -100;
    }

    ds18b20_convert_all(owb);
    // In this application all devices use the same resolution,
    // so use the first device to determine the delay
    ds18b20_wait_for_conversion(ds18b20_info);
    float temp = 0;
    ds18b20_read_temp(ds18b20_info, &temp);
    ESP_LOGI("one-wire", "  %d: %.3f\n", 0, temp);
    return temp;
}
