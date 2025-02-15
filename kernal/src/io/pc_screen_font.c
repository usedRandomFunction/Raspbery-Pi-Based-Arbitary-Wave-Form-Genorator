#include "io/pc_screen_font.h"

#include "io/framebuffer.h"

#include <stddef.h>

extern pc_screen_font_header _binary_data_font_psf_start;

pc_screen_font_header* current_font = &_binary_data_font_psf_start;

static pc_screen_font_header* s_last_font = NULL;
static int s_x_max = 0;    // Used to calculate size of text bounds
static int s_y_max = 0;    // Used to calculate size of text bounds


void pc_screen_font_darw(const char* str, uint32_t* x, uint32_t* y, int buffer)
{
    pc_screen_font_darw_ex(str, x, y, 0, get_display_width(), true, current_font, FRAMEBUFFER_RGB(255, 255, 255), FRAMEBUFFER_RGB(0,0,0), buffer);
}

void pc_screen_font_darw_ex(const char* str, uint32_t* x, uint32_t* y, uint32_t x_min, uint32_t x_max, 
    bool are_special_characters_enabled, pc_screen_font_header* font, display_color foreground, 
    display_color background, int buffer)
{
    if (font == NULL)
        font = current_font;

    s_last_font = font;
    s_y_max = 0;
    s_x_max = 0;

    int bytesperline = (font->width+7)/8;
    bool special_characters_enabled = are_special_characters_enabled;
    bool should_draw_pixels = ((background & 0xFF000000) == 0xFF000000) && ((foreground & 0xFF000000) == 0xFF000000);
    // If both forground and backgroud are fully transparent, then we shouldn't call set_display_pixel to save time

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

        for (int y_offset = 0 ; y_offset < font->height && should_draw_pixels; y_offset++)
        {
            uint64_t current_line_glyph = *(uint64_t*)glyph;

            for (int x_offset = 0 ; x_offset < font->width; x_offset++)
            {
                bool mask = ((current_line_glyph) & (1 << (8 -x_offset))) != 0;


                set_display_pixel(*x + x_offset, *y + y_offset, mask ? foreground : background, buffer);
            }

            glyph += bytesperline;
        }

        *x += font->width;

        if (*x > s_x_max)
            s_x_max = *x;
    }

    s_y_max = *y;
}

void pc_screen_font_get_text_bounds_bottom_right(uint32_t* x, uint32_t* y)
{

    if (s_last_font == NULL)        // I.e nothing has been drawn
    {
        *x = 0;
        *y = 0;
        return;
    }

    *y = s_y_max + s_last_font->height;
    *x = s_x_max;
}

void pc_screen_font_get_text_size_px(const char* str, uint32_t* x, uint32_t* y, uint32_t max_width, pc_screen_font_header* font)
{
    uint32_t x_pos = 0;
    uint32_t y_pos = 0;

    if (font == NULL)
        font = current_font;

    // When told to draw, but both colors are fully transparent, the function will not attempt to draw anything
    // as a further protection, the invalid buffer `-9999` is given. Dispite not drawing anything, it still keeps track of x, y and word warping
    pc_screen_font_darw_ex(str, &x_pos, &y_pos, 0, max_width, true, font, FRAMEBUFFER_RGBA(0, 0, 0, 255), FRAMEBUFFER_RGBA(0, 0, 0, 255), -9999);

    // So we can now do this

    pc_screen_font_get_text_bounds_bottom_right(x, y);
    // Since It starts at (0, 0) the bottom right (x, y) will also be (width, height) :3
}