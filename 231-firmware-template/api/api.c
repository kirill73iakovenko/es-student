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
#include "pi-task.h"


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

    device_info_t info;
    device_get_info(&info);

    printf("name: %s\n", DEVICE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", DEVICE_PROJECT);
    printf("repo: %s\n", DEVICE_REPO);
    printf("board: %s\n", info.board);
    printf("serial: %s\n", info.serial);

    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
           (unsigned)info.manufacturer,
           (unsigned)info.part,
           (unsigned)info.revision);

    printf("pico-sdk: %s\n", info.sdk_version);
}

static void cmd_uptime(const command_t *cmd)
{
    (void)cmd;
    printf("uptime: %llu ms\n", time_us_64() / 1000);
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

static void cmd_pi_start(const command_t *command)
{
    uint32_t terms = CALC_PI_DEFAULT_TERMS;

    // Проверяем количество аргументов
    if (command->argc > 1) {
        printf("error: usage pi_start [<terms>]\r\n");
        return;
    }

    // Если аргумент передан, проверяем его запись
    if (command->argc == 1) {
        if (!parse_u32(command->argv[0], &terms)) {
            printf("error: usage pi_start [<terms>]\r\n");
            return;
        }
    }

    // Проверяем допустимость числа членов
    if (terms == 0) {
        printf("error: terms must be greater than 0\r\n");
        return;
    }

    // Пытаемся запустить вычисление
    if (!pi_task_start(terms)) {
        printf("error: pi is busy\r\n");
        return;
    }

    printf("pi: started, %u terms\r\n", (unsigned)terms);
}

static void cmd_pi(const command_t *command)
{
    (void)command;

    double result;
    uint64_t elapsed_us;

    switch (pi_task_get_state()) {
    case PI_STATE_IDLE:
        printf("pi: not started\r\n");
        break;

    case PI_STATE_RUNNING:
        printf("pi: running\r\n");
        break;

    case PI_STATE_DONE:
        if (pi_task_get_result(&result, &elapsed_us)) {
            printf("pi: %.9f in %llu ms\r\n",
                   result,
                   (unsigned long long)(elapsed_us / 1000u));
        }
        break;
    }
}

static void cmd_profiling(const command_t *command)
{
    (void)command;

    bool found = false;

    for (uint32_t i = 0; i < PROFILING_STOPWATCH_COUNT; i++) {
        profiling_result_t result;

        if (profiling_get(i, &result)) {
            found = true;

            printf("%-10s count %-10u mean %.2f us  max %u us\n",
                   result.name,
                   (unsigned)result.count,
                   result.mean_us,
                   (unsigned)result.max_time_us);
        }
    }

    if (!found) {
        printf("profiling is disabled in this build\n");
    }
}

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
    printf("profiling: reset\r\n");
}

static const api_command_t commands[] = {
    { "info",            "device passport",                          cmd_info            },
    { "uptime",          "time since reset",                         cmd_uptime          },
    { "led_enable",      "turn led on",                          cmd_led_enable      },
    { "led_disable",     "turn led off",                         cmd_led_disable     },
    { "led_blink",       "blink the led",                            cmd_led_blink       },
    { "led_period",      "set led blink period", cmd_led_period } ,
    { "button",    "get button status",                   cmd_button    },
    { "pi_start", "start pi calculation", cmd_pi_start },
    { "pi",       "get pi result", cmd_pi       },
    { "profiling",       "get profiling results",                    cmd_profiling       },
    { "profiling_reset", "reset profiling results",                  cmd_profiling_reset },
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