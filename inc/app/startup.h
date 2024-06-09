#ifndef STARTUP_H
#define STARTUP_H

void PrintSystemSpecs();

// Prints info about some of the onbard clocks
// @param targetSpeed: pertage [0, 1] of the maximum clock speed
void SetupSystemClocks(float targetSpeed);

#endif