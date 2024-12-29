#include "io/pc_screen_font.h"

#include "io/framebuffer.h"

#include <stddef.h>

extern pc_screen_font_header _binary_data_font_psf_start;


pc_screen_font_header* current_font = &_binary_data_font_psf_start;

void pc_screen_font_darw(const char* str, uint32_t* x, uint32_t* y)
{
    pc_screen_font_darw_ex(str, x, y, 0, get_framebuffer_width(), true, current_font);
}

void pc_screen_font_darw_ex(const char* str, uint32_t* x, uint32_t* y, uint32_t x_min, uint32_t x_max, 
    bool are_special_characters_enabled, pc_screen_font_header* font)
{
    if (font == NULL)
        font = current_font;

    int bytesperline = (font->width+7)/8;
    bool special_characters_enabled = are_special_characters_enabled;
    
    for ( ; *str != '\0' ; str++)
    {
        if ((*x + font->width) >= x_max ||
            (*str == '\n' && special_characters_enabled))
        {
            *x = x_min;
            *y += font->height;
            continue;
        }

        if (*str == '\r' && special_characters_enabled)
        {
            *x = x_min;
            continue;
        } else if (*str == '\b' && special_characters_enabled)
        {
            uint32_t new_x = *x - font->width;
            if (new_x > *x || new_x < x_min) // If overflow or past boundarys
                new_x = x_min;

            *x = new_x;
        }

        if (*str == '\\' && special_characters_enabled)
        {
            special_characters_enabled = false;
            continue;
        }
        else
            special_characters_enabled = are_special_characters_enabled;

        
        int glyph_number = *((uint8_t*)str) < font->numglyph ? *((uint8_t*)str) : 0;
        uint8_t* glyph = &font->glyphs + glyph_number * font->bytesperglyph;

        for (int y_offset = 0 ; y_offset < font->height; y_offset++)
        {
            uint64_t current_line_glyph = *(uint64_t*)glyph;

            for (int x_offset = 0 ; x_offset < font->width; x_offset++)
            {
                bool mask = ((current_line_glyph) & (1 << (8 -x_offset))) != 0;

                if (mask)
                    set_framebuffer_pixel(*x + x_offset, *y + y_offset, 225, 225, 225);
                else
                    set_framebuffer_pixel(*x + x_offset, *y + y_offset, 0, 0, 0);
            }

            glyph += bytesperline;
        }

        *x += font->width;
    }
}