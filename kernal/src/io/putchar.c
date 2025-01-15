#include "io/putchar.h"

#include "io/pc_screen_font.h"
#include "io/framebuffer.h"
#include "lib/math.h"
#include "io/uart.h"

static uint32_t console_output_y = 0;
static uint32_t console_output_x = 0;

void putchar_on_frame_buffer_blanked()
{
    console_output_y = 0;
    console_output_x = 0;
}

int putchar(int ch)
{
    uart_putc((uint8_t)ch);

    if (is_frambuffer_initialized())
    {
        char buffer[2];
        buffer[0] = (char)ch;
        buffer[1] = 0;
        pc_screen_font_darw(buffer, &console_output_x, &console_output_y, 0); // TODO switch a draw char function

        if (console_output_y > CONSOLE_MAX_HEIGHT)  
        {
            uint32_t copy_area_start = CONSOLE_MAX_HEIGHT * 1 / 3; 
            copy_area_start /= current_font->height;
            copy_area_start *= current_font->height; // Round to lowwest part of line

            uint32_t last_line_bottom = console_output_y;
            last_line_bottom /= current_font->height;
            last_line_bottom *= current_font->height;

            uint32_t new_last_line_bottom = last_line_bottom - copy_area_start;

            display_screen_copy(0, copy_area_start,                         // Copy area Start x, y
                get_display_width(), last_line_bottom - copy_area_start,    // Copy area size x, y
                0, 0, 0);                                                   // Paste area Start x, y and buffer ID

            display_fill_rect(0, new_last_line_bottom,                      // x0, y0
                get_display_width(), get_display_height(),                  // x1, y1
                FRAMEBUFFER_RGB(0, 0, 0), 0);                               // color and buffer

            console_output_y = new_last_line_bottom;
        }
    }

    return 0;
}