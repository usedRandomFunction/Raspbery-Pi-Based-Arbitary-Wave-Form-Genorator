#ifndef TIMING_H
#define TIMING_H


#include <stddef.h>
#include <stdint.h>

// Loops cycles times runing nop on each cycle
// @param cycles The number of cycles
void wait_cycles(size_t cycles);

// halts the system for given number of microseconds
// @param microseconds The delay time
void delay_microseconds(size_t microseconds);

// halts the system for given number of milliseconds
// @param milliseconds The delay time
void delay_milliseconds(size_t milliseconds);

// Returns the value of cntfrq_el0
// @return cntfrq_el0
uint32_t get_timer_freqency();

// Returns the value of cntpct_el0,
// @return cntpct_el0,
uint64_t get_timer_count();

#endif