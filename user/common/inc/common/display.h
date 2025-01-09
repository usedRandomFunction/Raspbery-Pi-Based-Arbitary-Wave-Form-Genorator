#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>


struct pc_screen_font_header{
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
    uint8_t glyphs;
};

typedef struct pc_screen_font_header pc_screen_font_header;

typedef uint32_t display_color;

#define FRAMEBUFFER_RGBA(r, g, b, a) ((r) | (g << 8) | (b << 16) | (a << 24))
#define FRAMEBUFFER_RGB(r, g, b) (FRAMEBUFFER_RGBA(r, g, b, 0))


// Sets the given pixel to the given RGB value
// @param x The X coordinate 
// @param y The Y coordinate 
// @param The color (including alpha) to write
// @param buffer The ID of the target frame buffer, if -1 is given the active frame buffer will be used
void set_display_pixel(uint32_t x, uint32_t y, display_color color, int buffer);

// Sets the color of the color of pixels in the rect
// @param x0 The X coordinate of the top left corner
// @param y0 The Y coordinate of the top left corner
// @param x1 The X coordinate of the bottom right corner
// @param y1 The Y coordinate of the bottom right corner
// @param The color (including alpha) to write
// @param buffer The ID of the target frame buffer, if -1 is given the active frame buffer will be used
void display_fill_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, display_color color, int buffer);

// Copys [size_x, size_y] pixels from `data` to (x, y) as the top left coner.
// @param x Left most x coordinate 
// @param size_x Nubmer of pixels to copy in the x-axis
// @param y Top most y coordinate 
// @param size_y Nubmer of pixels to copy in the y-axis
// @param data The buffer to copy from
// @param pixels_per_line The number of pixels per line in the data buffer, if `0` if given it will defult to size_x
// @param buffer The ID of the target frame buffer, if -1 is given the active frame buffer will be used
void copy_to_display(uint32_t x, uint32_t size_x, uint32_t y, uint32_t size_y, display_color* data, uint32_t pixels_per_line, int buffer);

// Draws the given string to the frame buffer at (x, y) top left wrapped
// @param str A pointer to a null termnated string
// @param x The X coordinate, is set to the coordinate of the end of the text block
// @param y The Y coordinate, is set to the coordinate of the end of the text block
// @param x_min The value x will be set to on a new line
// @param x_max The value used by word wraping
// @param are_special_characters_enabled, enables or dissables special characters i.e '\n'
// @param font Font to be used
// @param foreground Foreground color
// @param background Background color
// @param buffer The ID of the target frame buffer, if -1 is given the active frame buffer will be used
void display_draw_string(const char* str, uint32_t* x, uint32_t* y, uint32_t x_min, uint32_t x_max, 
    bool are_special_characters_enabled, pc_screen_font_header* font, display_color foreground, 
    display_color background, int buffer);

// Get the width in pixels of the display
// @returns display height in pixels
uint32_t get_display_height();

// Get the width in pixels of the display
// @returns displaywidth in pixels
uint32_t get_display_width();

// Sets / gets which frame buffer is currently displayed on the screen
// @param buffer The new buffer be displayed, a value of -1, results in no action being taken, note that -2 is a readonly buffer used by printf and putchar,
// @return The active frame buffe at the end of the operation
int active_framebuffer(int buffer);

// Sets / gets how many frame buffesr are active / enabled
// @param nbuffers How many frame buffers are requested to exist, a value of -1, results in no action being taken
// @return The number of frame buffers that are avalible, at the end of the operation
int request_frame_buffers(int nbuffers);

#endif