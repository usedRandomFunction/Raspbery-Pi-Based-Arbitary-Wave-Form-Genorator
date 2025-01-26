#ifndef RUNTIME_CONFIG_EDITING_H
#define RUNTIME_CONFIG_EDITING_H

#include <stdint.h>

// Sets the over scan values of the display
// @param top Overscan in pixels
// @param bottom Overscan in pixels
// @param left Overscan in pixels
// @param right Overscan in pixels
// @note This function does not edit system.cfg, only the values stored in ram and is only to be used to test values
void set_display_overscan(uint32_t top, uint32_t bottom, uint32_t left, uint32_t right);

#endif