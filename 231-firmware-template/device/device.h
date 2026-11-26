#pragma once


#include <stdint.h>
#include "pico/unique_id.h"



#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.1.0"

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/kirill73iakovenko/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif



typedef struct {
    const char *board;
    char serial[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    uint32_t manufacturer;
    uint32_t part;
    uint32_t revision;
    const char *sdk_version;
} device_info_t;

void device_get_info(device_info_t *info);





