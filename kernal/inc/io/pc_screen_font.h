#ifndef PC_SCREEN_FONT_H
#define PC_SCREEN_FONT_H

#include "io/framebuffer.h"

#include <stdbool.h>
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

extern pc_screen_font_header* current_font;

// Draws the given string to the frame buffer at (x, y) top left wrapped
// @param str A pointer to a null termnated string
// @param x The X coordinate, is set to the coordinate of the end of the text block
// @param y The Y coordinate, is set to the coordinate of the end of the text block
// @param buffer The ID of the target frame buffer, 0 to nbuffers - 1,
void pc_screen_font_darw(const char* str, uint32_t* x, uint32_t* y, int buffer);

// Draws the given string to the frame buffer at (x, y) top left wrapped
// @param str A pointer to a null termnated string
// @param x The X coordinate, is set to the coordinate of the end of the text block
// @param y The Y coordinate, is set to the coordinate of the end of the text block
// @param x_min The value x will be set to on a new line
// @param x_max The value used by word wraping
// @param are_special_characters_enabled, enables or dissables special characters i.e '\n'
// @param font Font to be used, if none are given current_font is used;
// @param foreground Foreground color
// @param background Background color
// @param buffer The ID of the target frame buffer, 0 to nbuffers - 1,
void pc_screen_font_darw_ex(const char* str, uint32_t* x, uint32_t* y, uint32_t x_min, uint32_t x_max, 
    bool are_special_characters_enabled, pc_screen_font_header* font, display_color foreground, 
    display_color background, int buffer);

// Returns the x,y coordinates that define the bottom right of
// a bounding box to fit the last text drawn by pc_screen_font_darw_ex.
// @param x The X coordinate
// @param y The Y coordinate
void pc_screen_font_get_text_bounds_bottom_right(uint32_t* x, uint32_t* y);

// Calcuates the size of given text in pixels using the given font.
// The function also allows for a max of a any line to be given.
// @param str A pointer to a null termnated string
// @param x To be set to the width in px
// @param y To be set to the height in px
// @param max_width, the max allowed width of a line in px
// @param font Font to be used, if none are given current_font is used;
void pc_screen_font_get_text_size_px(const char* str, uint32_t* x, uint32_t* y, uint32_t max_width, pc_screen_font_header* font);

#endif