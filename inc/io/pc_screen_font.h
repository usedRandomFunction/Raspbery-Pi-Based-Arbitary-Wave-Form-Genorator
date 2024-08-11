#ifndef PC_SCREEN_FONT_H
#define PC_SCREEN_FONT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
void pc_screen_font_darw(const char* str, uint32_t* x, uint32_t* y);

// Draws the given string to the frame buffer at (x, y) top left wrapped
// @param str A pointer to a null termnated string
// @param x The X coordinate, is set to the coordinate of the end of the text block
// @param y The Y coordinate, is set to the coordinate of the end of the text block
// @param x_min The value x will be set to on a new line
// @param x_max The value used by word wraping
// @param are_special_characters_enabled, enables or dissables special characters i.e '\n'
void pc_screen_font_darw_ex(const char* str, uint32_t* x, uint32_t* y, uint32_t x_min, uint32_t x_max, bool are_special_characters_enabled);

#ifdef __cplusplus
}
#endif


#endif