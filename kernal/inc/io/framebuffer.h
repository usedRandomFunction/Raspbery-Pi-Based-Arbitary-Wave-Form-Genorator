#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdbool.h>
#include <stdint.h>



extern bool is_frambuffer_initialized;

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

// Copys the given area to the past area top left first
// @param copy_area_start_x The X coordinate of the start of the copy region
// @param copy_area_start_y The Y coordinate of the start of the copy region
// @param copy_area_size_x The width of the copy area
// @param copy_area_size_y The height of the copy area
// @param paste_area_start_x The X coordinate of the start of the copy region
// @param paste_area_start_y The Y coordinate of the start of the copy region
void framebuffer_screen_copy(uint32_t copy_area_start_x, uint32_t copy_area_start_y, uint32_t copy_area_size_x, uint32_t copy_area_size_y, uint32_t paste_area_start_x, uint32_t paste_area_start_y);

// Sets the color of the color of pixels in the rect
// @param x0 The X coordinate of the top left corner
// @param y0 The Y coordinate of the top left corner
// @param x1 The X coordinate of the bottom right corner
// @param y1 The Y coordinate of the bottom right corner
// @param r The red channel value
// @param g The green channel value
// @param b The blue channel value
void framebuffer_fill_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t r, uint8_t g, uint8_t b);

// Get the width in pixels of the frame buffer
// @returns frame buffer height in pixels
uint32_t get_framebuffer_height();

// Get the width in pixels of the frame buffer
// @returns frame buffer width in pixels
uint32_t get_framebuffer_width();



#endif