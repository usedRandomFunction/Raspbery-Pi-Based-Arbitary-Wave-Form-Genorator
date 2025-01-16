#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t display_color;

#define FRAMEBUFFER_RGBA(r, g, b, a) ((r) | (g << 8) | (b << 16) | (a << 24))
#define FRAMEBUFFER_RGB(r, g, b) (FRAMEBUFFER_RGBA(r, g, b, 0))

// Initializes the frame buffe
// @return True is success, False is failure
bool initialize_framebuffer();

// Sets / gets which frame buffer is currently displayed on the screen
// @param buffer The new buffer be displayed, a value of -1, results in no action being taken
// @return The active frame buffe at the end of the operation
int active_framebuffer(int buffer);

// Sets / gets how many frame buffesr are active / enabled
// @param nbuffers How many frame buffers are requested to exist, a value of -1, results in no action being taken
// @return The number of frame buffers that are avalible, at the end of the operation
int request_frame_buffers(int nbuffers);

// Sets the given pixel to the given RGB value
// @param x The X coordinate 
// @param y The Y coordinate 
// @param The color (including alpha) to write
// @note This edits the VC frame buffer, not the virtual frame buffers used by other functions
void set_framebuffer_pixel(uint32_t x, uint32_t y, display_color color);

// Sets the given pixel to the given RGB value
// @param x The X coordinate 
// @param y The Y coordinate 
// @param The color (including alpha) to write
// @param buffer The ID of the target frame buffer, 0 to nbuffers - 1,
void set_display_pixel(uint32_t x, uint32_t y, display_color color, int buffer);

// Copys the given area to the past area top left first
// @param copy_area_start_x The X coordinate of the start of the copy region
// @param copy_area_start_y The Y coordinate of the start of the copy region
// @param copy_area_size_x The width of the copy area
// @param copy_area_size_y The height of the copy area
// @param paste_area_start_x The X coordinate of the start of the copy region
// @param paste_area_start_y The Y coordinate of the start of the copy region
// @note This edits the VC frame buffer, not the virtual frame buffers used by other functions
void framebuffer_screen_copy(uint32_t copy_area_start_x, uint32_t copy_area_start_y, uint32_t copy_area_size_x, uint32_t copy_area_size_y, uint32_t paste_area_start_x, uint32_t paste_area_start_y);

// Copys the given area to the past area top left first
// @param copy_area_start_x The X coordinate of the start of the copy region
// @param copy_area_start_y The Y coordinate of the start of the copy region
// @param copy_area_size_x The width of the copy area
// @param copy_area_size_y The height of the copy area
// @param paste_area_start_x The X coordinate of the start of the copy region
// @param paste_area_start_y The Y coordinate of the start of the copy region
// @param buffer The ID of the target frame buffer, 0 to nbuffers - 1,
void display_screen_copy(uint32_t copy_area_start_x, uint32_t copy_area_start_y, uint32_t copy_area_size_x, uint32_t copy_area_size_y, uint32_t paste_area_start_x, uint32_t paste_area_start_y, int buffer);

// Sets the color of the color of pixels in the rect
// @param x0 The X coordinate of the top left corner
// @param y0 The Y coordinate of the top left corner
// @param x1 The X coordinate of the bottom right corner
// @param y1 The Y coordinate of the bottom right corner
// @param The color (including alpha) to write
// @note This edits the VC frame buffer, not the virtual frame buffers used by other functions
void framebuffer_fill_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, display_color color);

// Sets the color of the color of pixels in the rect
// @param x0 The X coordinate of the top left corner
// @param y0 The Y coordinate of the top left corner
// @param x1 The X coordinate of the bottom right corner
// @param y1 The Y coordinate of the bottom right corner
// @param The color (including alpha) to write
// @param buffer The ID of the target frame buffer, 0 to nbuffers - 1,
void display_fill_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, display_color color, int buffer);

// Sets the over scan values of the display
// @param top Overscan in pixels
// @param bottom Overscan in pixels
// @param left Overscan in pixels
// @param right Overscan in pixels
void set_display_overscan(uint32_t top, uint32_t bottom, uint32_t left, uint32_t right);

// Get the width in pixels of the display
// @returns display height in pixels
uint32_t get_display_height();

// Get the width in pixels of the display
// @returns displaywidth in pixels
uint32_t get_display_width();

// @return True if frmae buffer is initialized
bool is_frambuffer_initialized();

// Resets the frame buffers after a app exits
void framebuffer_on_user_app_exit();

#endif