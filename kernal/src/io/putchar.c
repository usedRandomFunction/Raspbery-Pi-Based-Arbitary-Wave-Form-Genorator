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
            uint32_t copy_area_start_y = CONSOLE_MAX_HEIGHT * 1 / 3; 

            if (copy_area_start_y % current_font->height)   // Round up
                copy_area_start_y += current_font->height - (copy_area_start_y % current_font->height);

            uint32_t bottom_of_last_line = console_output_y;// + current_font->height;
            uint32_t copy_area_size_y = bottom_of_last_line - copy_area_start_y;

            uint32_t new_bottom_of_last_line = copy_area_size_y;//bottom_of_last_line - copy_area_start_y;

            display_screen_copy(0, copy_area_start_y,           // Copy start X,Y
                get_display_width() - 1, copy_area_size_y,      // Copy size X,Y
                0, 0, 0);                                       // Paste start X,Y and buffer ID


            display_fill_rect(0, new_bottom_of_last_line,           // x0, y0
                get_display_width() - 1, get_display_height() - 1,  // x1 y1
                FRAMEBUFFER_RGB(0, 0, 0), 0);

            console_output_y = new_bottom_of_last_line;// - current_font->height;
        }
    }

    return 0;
}