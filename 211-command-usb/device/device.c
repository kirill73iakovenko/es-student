#include "device.h"

#include <stdio.h>
#include <stddef.h>
#include "pico/unique_id.h"
#include "hardware/regs/addressmap.h"
#include "pico/version.h"
#include "hardware/regs/sysinfo.h"

void device_info(void)
{   
    char board_id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    pico_get_unique_board_id_string(board_id, sizeof(board_id));
    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;
    uint32_t manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> 
        SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    uint32_t part = (id & SYSINFO_CHIP_ID_PART_BITS) >> 
        SYSINFO_CHIP_ID_PART_LSB;
    uint32_t revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> 
        SYSINFO_CHIP_ID_REVISION_LSB;
    
    
    

    printf("project: %s\n", DEVICE_PROJECT);
    printf("repo: %s\n", DEVICE_REPO);
    printf("board: %s\n", DEVICE_BOARD);
    printf("serial: %s\n", board_id);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", 
        manufacturer, part, revision);
    printf("pico-sdk: %s\n", PICO_SDK_VERSION_STRING);
}

struct info_t device_card = {
    .version = 0x00020300,
    .name = "211-cmd-usb",
    .revision = 1   
};

void dev_info(void){

    printf("%-13s %-10s %5s %6s %10s\n",
           "struct", "address", "size", "offset", "value");

    printf("- %-13s 0x%08x %5u %6u 0x%08x\n",
           "version",
           &device_card.version,
           sizeof(device_card.version),
           offsetof(struct info_t, version),
           device_card.version);
    printf("- %-13s 0x%08x %5u %6u 0x%08x\n",
           "name",
           &device_card.name,
           sizeof(device_card.name),
           offsetof(struct info_t, name),
           device_card.name);

    printf("- %-13s 0x%08x %5u %6u 0x%08x\n",
           "revision",
           &device_card.revision,
           sizeof(device_card.revision),
           offsetof(struct info_t, revision),
           device_card.revision);
    
    unsigned fields = sizeof(device_card.version) 
    + sizeof(device_card.name) + sizeof(device_card.revision);
    unsigned real_size = sizeof(device_card);
    unsigned padding = real_size - fields;
    printf("fields %u, sizeof %u, padding %u\n", fields, real_size, padding);
}
    

