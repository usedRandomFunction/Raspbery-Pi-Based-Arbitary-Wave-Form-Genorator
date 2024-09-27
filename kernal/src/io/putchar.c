#include "io/putchar.h"

#include "io/pc_screen_font.h"
#include "io/framebuffer.h"
#include "lib/math.h"
#include "io/uart.h"

static uint32_t console_output_y;
static uint32_t console_output_x;

int putchar(int ch)
{
    uart_putc((uint8_t)ch);

    if (is_frambuffer_initialized)
    {
        char buffer[2];
        buffer[0] = (char)ch;
        buffer[1] = 0;
        pc_screen_font_darw(buffer, &console_output_x, &console_output_y); // TODO switch a draw char function

        if (console_output_y > CONSOLE_MAX_HEIGHT)  
        {
            uint32_t copy_area_start = CONSOLE_MAX_HEIGHT * 1 / 3; 
            copy_area_start /= current_font->height;
            copy_area_start *= current_font->height; // Round to lowwest part of line

            uint32_t last_line_bottom = console_output_y;
            last_line_bottom /= current_font->height;
            last_line_bottom *= current_font->height;

            uint32_t new_last_line_bottom = last_line_bottom - copy_area_start;

            framebuffer_screen_copy(0, copy_area_start,                         // Copy area Start x, y
                get_framebuffer_width(), last_line_bottom - copy_area_start,    // Copy area size x, y
                0, 0);                                                          // Paste area Start x, y

            framebuffer_fill_rect(0, new_last_line_bottom,                      // Fill in the old area with black
                get_framebuffer_width(), get_framebuffer_height(),
                0, 0, 0); // R G B

            console_output_y = new_last_line_bottom;
        }
    }

    return 0;
}

void putchar_init_values()
{
    console_output_y = 0;
    console_output_x = 0;
}