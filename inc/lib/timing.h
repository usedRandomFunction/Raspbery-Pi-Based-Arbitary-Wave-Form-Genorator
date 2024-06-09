#ifndef TIMING_H
#define TIMING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Loop <delay> times in a way that the compiler won't optimize away
// @param count The number of times to loop
static inline void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}


#ifdef __cplusplus
}
#endif

#endif