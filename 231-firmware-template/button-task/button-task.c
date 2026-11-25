#include "button-task.h"
#include "led-task.h"
#include "pico/stdlib.h"

#include <stdint.h>
#include <stdbool.h>

#define BUTTON_DEBOUNCE_US 20000u
#define PICO_BUTTON_PIN 15


static button_state_t button_state = RELEASED;
static uint64_t start_time_us = 0;
static uint32_t button_count = 0;

void button_task_init(void)
{
    gpio_init(PICO_BUTTON_PIN);
    gpio_set_dir(PICO_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(PICO_BUTTON_PIN);

    button_state = RELEASED;
    start_time_us = 0;
    button_count = 0;
}

void button_task_handle(void)
{
    // Читаем состояние кнопки
    bool pressed = !gpio_get(PICO_BUTTON_PIN);

    // Читаем текущее время в микросекундах
    uint64_t now_us = time_us_64();

    switch (button_state) {
    case RELEASED:
        if (pressed) {
            start_time_us = now_us;
            button_state = PRESSING;
        }
        break;

    case PRESSING:
        if (!pressed) {
            button_state = RELEASED;
        } else if (now_us - start_time_us >= BUTTON_DEBOUNCE_US) {
            button_state = PRESSED;
            button_count++;
            led_task_next_state();
        }
        break;

    case PRESSED:
        if (!pressed) {
            start_time_us = now_us;
            button_state = RELEASING;
        }
        break;

    case RELEASING:
        if (pressed) {
            button_state = PRESSED;
        } else if (now_us - start_time_us >= BUTTON_DEBOUNCE_US) {
            button_state = RELEASED;
        }
        break;
    }
}

bool button_task_is_pressed(void)
{
    return button_state == PRESSED ||
           button_state == RELEASING;
}

uint32_t button_task_get_press_count(void)
{
    return button_count;
}