#ifndef STARTUP_H
#define STARTUP_H

#include <stdint.h>

void PrintSystemSpecs();

// Prints info about some of the onbard clocks and sets the speed
// @param targetSpeed: pertage [0, 1] of the maximum clock speed
void SetupSystemClocks(float targetSpeed);

// Prints info about the given onbard clock and sets the speed
// @param targetSpeed: pertage [0, 1] of the maximum clock speed
void PrintMaxiumClockSpeedAndSet(uint32_t clock_id, const char* name, float targetSpeed);

#endif