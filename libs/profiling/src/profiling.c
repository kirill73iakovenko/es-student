#include "profiling/profiling.h"
#include "pico/stdlib.h"


#if PROFILING_ENABLED

// Внутреннее устройство секундомера
typedef struct {
    const char *name;
    uint32_t start_us;
    uint32_t count;
    uint32_t sum_us;
    uint32_t max_us;
    bool running;
} profiling_stopwatch_t;


static profiling_stopwatch_t stopwatches[PROFILING_STOPWATCH_COUNT];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_STOPWATCH_COUNT || name == NULL) {
        return;
    }

    stopwatches[id].name = name;
    stopwatches[id].start_us = 0;
    stopwatches[id].count = 0;
    stopwatches[id].sum_us = 0;
    stopwatches[id].max_us = 0;
    stopwatches[id].running = false;
}




void profiling_start(uint32_t id)
{
    if (id >= PROFILING_STOPWATCH_COUNT) {
        return;
    }

    profiling_stopwatch_t *sw = &stopwatches[id];

    if (sw->name == NULL || sw->running) {
        return;
    }

    sw->start_us = time_us_32();
    sw->running = true;
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_STOPWATCH_COUNT) {
        return;
    }

    profiling_stopwatch_t *sw = &stopwatches[id];

    if (!sw->running) {
        return;
    }

    uint32_t elapsed_us = time_us_32() - sw->start_us;

    sw->running = false;
    sw->count++;
    sw->sum_us += elapsed_us;

    if (elapsed_us > sw->max_us) {
        sw->max_us = elapsed_us;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= PROFILING_STOPWATCH_COUNT || result == NULL) {
        return false;
    }

    profiling_stopwatch_t *sw = &stopwatches[id];

    if (sw->name == NULL) {
        return false;
    }

    result->name = sw->name;
    result->count = sw->count;
    result->max_time_us = sw->max_us;

    if (sw->count == 0) {
        result->mean_us = 0.0;
    } else {
        result->mean_us = (double)sw->sum_us / sw->count;
    }

    return true;
}

void profiling_reset(void)
{
    for (uint32_t i = 0; i < PROFILING_STOPWATCH_COUNT; i++) {
        stopwatches[i].start_us = 0;
        stopwatches[i].count = 0;
        stopwatches[i].sum_us = 0;
        stopwatches[i].max_us = 0;
        stopwatches[i].running = false;
    }
}

#endif