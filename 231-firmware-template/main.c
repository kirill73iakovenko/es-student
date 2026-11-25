#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "led.h"
#include "profiling.h"
#include "api.h"
#include "stdio-text-protocol.h"


// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
const uint BLINK_HALF_PERIOD_MS = 500;

uint64_t last_toggle_us = 0;

void blink(void)
{
    uint64_t now_us = time_us_64();

    if (now_us - last_toggle_us >= BLINK_HALF_PERIOD_MS * 1000)
    {
        last_toggle_us = now_us;
        led_toggle();
    }
}





int main(){ 

    stdio_text_protocol_init();

    led_init();
    profiling_init();
    while (1)
    {
        profiling_iteration();
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
            {
                api_handle(command);
            }
    }
   }

