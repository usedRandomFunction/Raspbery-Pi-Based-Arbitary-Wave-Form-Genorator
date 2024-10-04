#ifndef TIMING_H
#define TIMING_H


#include <stddef.h>
#include <stdint.h>

// // Loop <delay> times in a way that the compiler won't optimize away
// // @param count The number of times to loop
// static inline void delay(int32_t count)
// {
// 	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
// 		 : "=r"(count): [count]"0"(count) : "cc");
// }

// Loops cycles times runing nop on each cycle
// @param cycles The number of cycles
void wait_cycles(size_t cycles);

// halts the system for given number of microseconds
// @param microseconds The delay time
void delay_microseconds(size_t microseconds);

// halts the system for given number of milliseconds
// @param milliseconds The delay time
void delay_milliseconds(size_t milliseconds);


#endif