#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

// As the name implys, intinializes the random number genorator
void initialize_random();

// Genorates Random number
// @param min The minium number
// @param max The maxium number
// @return number
uint32_t random(uint32_t min, uint32_t max);

#endif