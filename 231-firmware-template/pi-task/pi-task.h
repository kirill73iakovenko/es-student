#pragma once

#include <stdint.h>
#include <stdbool.h>

bool pi_task_start(uint32_t n);
void pi_task_handle(void);


typedef enum {
    PI_STATE_IDLE,     // Не запускался
    PI_STATE_RUNNING,  // Считает
    PI_STATE_DONE,     // Готово
} pi_state_t;
pi_state_t pi_task_get_state(void);
bool pi_task_get_result(double *result, uint64_t *elapsed_us);