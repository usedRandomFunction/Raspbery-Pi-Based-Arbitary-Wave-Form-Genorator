#ifndef CLOCKS_H
#define CLOCKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib/memory.h"

#include <stdint.h>

uint32_t get_clock_rate_messured(uint32_t clock_id);
uint32_t get_maximum_clock_rate(uint32_t clock_id);
uint32_t get_minimum_clock_rate(uint32_t clock_id);
uint32_t get_clock_rate(uint32_t clock_id);

// Sets the given clock to the given rate
// Returns the value return for the rate set or zero if failer
uint32_t set_clock_rate(uint32_t clock_id, uint32_t rate);

#ifdef __cplusplus
}
#endif

#endif