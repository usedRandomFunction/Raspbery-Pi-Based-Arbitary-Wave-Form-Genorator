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
uint32_t get_timer_frequency();

// Returns the value of cntpct_el0,
// @return cntpct_el0,
uint64_t get_timer_count();

// Enables nCNTPNSIRQ in CORE_0_TIMER_IRQ_CTRL
// must be called before runing set_timer_interrupt
void enable_timer_interrupt();

// Sets CNTP_CTL_EL0, and CNTP_TVAL_EL0 to have a interrupt in the future
// @param microseconds The delay befor it activates
// @note you must call enable_timer_interrupt before this function
void set_timer_interrupt(uint32_t microseconds);

// Sets CNTP_CTL_EL0 to 0
void dissable_timer_interrupt();

#endif