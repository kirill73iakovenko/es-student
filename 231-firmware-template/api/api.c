#include "api.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>   
#include <ctype.h>     
#include <errno.h>   

#include "pico/time.h"

#include "log.h"
#include "device.h"
#include "profiling.h"
#include "led-task.h"
#include "button-task.h"


#ifndef CALC_PI_DEFAULT_TERMS
#define CALC_PI_DEFAULT_TERMS 1000000u
#endif

static bool parse_u32(const char *text, uint32_t *value)
{
    
    if (!isdigit((unsigned char)text[0])) {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX) {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}


typedef void (*api_callback_t)(const command_t *command);

typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;
static void cmd_info(const command_t *cmd)
{
    (void)cmd;
    device_info();
}

static void cmd_uptime(const command_t *cmd)
{
    (void)cmd;
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static void cmd_calc_pi(const command_t *command)
{
    uint32_t terms = CALC_PI_DEFAULT_TERMS;

    if (command->argc > 1) {
        printf("error: usage calc_pi [terms]\r\n");
        return;
    }

    if (command->argc == 1) {
        uint32_t v;
        if (!parse_u32(command->argv[0], &v)) {
            printf("error: usage calc_pi [terms]\r\n");
            return;
        }
        if (v == 0) {
            printf("error: calc_pi terms must be positive\r\n");
            return;
        }
        terms = v;
    }

    
    uint64_t start_us = time_us_64();

    double sum  = 0.0;
    double sign = 1.0;
    for (uint32_t k = 0; k < terms; k++) {
        sum += sign / (2.0 * k + 1.0);
        sign = -sign;
    }

    uint64_t spent_us = time_us_64() - start_us;

    
    printf("pi: %.9f\r\n", 4.0 * sum);
    printf("time: %llu ms\r\n", (unsigned long long)(spent_us / 1000));
}

static void cmd_main_time_exec(const command_t *cmd)
{
    (void)cmd;
    printf("iteration avg %.2f us, max %u us\r\n",
           profiling_avg_us(), (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t *cmd)
{
    (void)cmd;
    profiling_reset_max();
    printf("max reset\r\n");
}

static void print_led_state(void)
{
    led_state_t state = led_task_get_state();
    const char *state_str;

    switch (state) {
    case LED_STATE_OFF:
        state_str = "off";
        break;

    case LED_STATE_ON:
        state_str = "on";
        break;

    case LED_STATE_BLINK:
        state_str = "blink";
        break;

    default:
        state_str = "unknown";
        break;
    }

    uint32_t period_ms = led_task_get_period_ms();

    printf("led: %s, period: %u ms\r\n",
           state_str, (unsigned int)period_ms);
}

static void cmd_led_enable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_ON);
    print_led_state();

}

static void cmd_led_disable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_OFF);
    print_led_state();

}

static void cmd_led_blink(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    // годится ли такой период, решает задача светодиода
    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    // ответ: состояние и период светодиода
    print_led_state();
}

static void cmd_button(const command_t *command)
{
    (void)command;
    bool pressed = button_task_is_pressed();
    uint32_t count = button_task_get_press_count();
    printf("button: %s, presses: %u\r\n",
           pressed ? "pressed" : "released", (unsigned int)count);
}


static const api_command_t commands[] = {
    { "info",            "device passport",                          cmd_info            },
    { "uptime",          "time since reset",                         cmd_uptime          },
    { "calc_pi",         "calculate pi: calc_pi [terms]",            cmd_calc_pi         },
    { "main_time_exec",  "superloop iteration: average and maximum", cmd_main_time_exec  },
    { "main_time_reset", "reset maximum iteration",                  cmd_main_time_reset },
    { "led_enable",      "turn led on",                          cmd_led_enable      },
    { "led_disable",     "turn led off",                         cmd_led_disable     },
    { "led_blink",       "blink the led",                            cmd_led_blink       },
    { "led_period",      "set led blink period", cmd_led_period } ,
    { "button",    "get button status",                   cmd_button    },
};

static const uint32_t command_count =
    sizeof(commands) / sizeof(commands[0]);



void api_handle(const command_t *cmd)
{
    
    if (cmd->truncated) {
        printf("error: command longer than 63 characters or 4 arguments\r\n");
        return;
    }

    
    if (strcmp(cmd->name, "help") == 0) {
        printf("%-16s %s\r\n", "help", "list of commands");
        for (uint32_t i = 0; i < command_count; i++) {
            printf("%-16s %s\r\n", commands[i].name, commands[i].help);
        }
        return;
    }

    
    for (uint32_t i = 0; i < command_count; i++) {
        if (strcmp(cmd->name, commands[i].name) == 0) {
            commands[i].callback(cmd);
            return;
        }
    }

    printf("error: unknown command '%s', try help\r\n", cmd->name);
}