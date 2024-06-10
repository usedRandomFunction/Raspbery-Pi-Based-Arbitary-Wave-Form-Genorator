#ifndef CLOCKS_H
#define CLOCKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lib/memory.h"

#include <stdint.h>

// Gets messure the frequency of a given clock
// @param clock_id the clock id for the clock to get the rate
// @return The frequency in hertz, or zero if failed
uint32_t get_clock_rate_messured(uint32_t clock_id);

// Gets the maxium clock rate setting of a given clock
// @param clock_id the clock id for the clock to get the rate
// @return The frequency in hertz, or zero if failed
uint32_t get_maximum_clock_rate(uint32_t clock_id);

// Gets the minimum clock rate setting of a given clock
// @param clock_id the clock id for the clock to get the rate
// @return The frequency in hertz, or zero if failed
uint32_t get_minimum_clock_rate(uint32_t clock_id);

// Gets the current clock rate setting of a given clock
// @param clock_id the clock id for the clock to get the rate
// @return The frequency in hertz, or zero if failed
uint32_t get_clock_rate(uint32_t clock_id);

// Sets the given clock to the given rate
// @param clock_id the clock id that to be set
// @param rate the frequency in hertz to set the clock to
// @return The frequency in hertz that it was set to, or zero if failed
uint32_t set_clock_rate(uint32_t clock_id, uint32_t rate);

// Sets the given clock to the given rate only using the given functions for memory allocation
// @param clock_id the clock id that to be set
// @param rate the frequency in hertz to set the clock to
// @param _malloc A pointer to a alligned_alloc function
// @param _free A pointer to a free function
// @return The frequency in hertz that it was set to, or zero if failed
uint32_t set_clock_rate_given_alloc_functions(uint32_t clock_id, uint32_t rate, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free);

#ifdef __cplusplus
}
#endif

#endif