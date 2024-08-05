#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the frame buffer and attempts to set the width and height
// @param target_width The width we want to use
// @param target_height The target height we want to use
// @return True is success, False is failure
bool initialize_framebuffer(uint32_t target_width, uint32_t target_height);

// Sets the given pixel to the given RGB value
// @param x The X coordinate 
// @param y The Y coordinate 
// @param r The red channel value
// @param g The green channel value
// @param b The blue channel value
void set_framebuffer_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);

// Get the width in pixels of the frame buffer
// @returns frame buffer height in pixels
uint32_t get_framebuffer_height();

// Get the width in pixels of the frame buffer
// @returns frame buffer width in pixels
uint32_t get_framebuffer_width();

#ifdef __cplusplus
}
#endif

#endif