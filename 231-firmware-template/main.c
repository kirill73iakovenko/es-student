#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "led-task.h"
#include "profiling.h"
#include "api.h"
#include "stdio-text-protocol.h"


int main(){ 

    stdio_text_protocol_init();
    led_task_init();
    profiling_init();
    
    while (1)
    {
        profiling_iteration();
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
            {
                api_handle(command);
            }
        led_task_handle();
    }
   }

