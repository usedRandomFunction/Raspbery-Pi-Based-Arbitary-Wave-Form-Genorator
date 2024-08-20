#include "io/pc_screen_font.h"
#include "io/propertyTags.h"
#include "io/framebuffer.h"
#include "app/startup.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/math.h"
#include "io/gpio.h"
#include "io/uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern pc_screen_font_header _binary_data_font_psf_start; 

#include "io/printf.h"

int main()
{
    current_font = &_binary_data_font_psf_start;
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    uint32_t height = get_framebuffer_height();
    uint32_t width = get_framebuffer_width();

    for (int y = 0 ; y < height; y++)
    {
        for (int x = 0 ; x < width; x++)
        {
            set_framebuffer_pixel(x, y, 0, 0, 0);
        }
    }

    int x = 0;
    int y = 0;


    printf("hello world, %x", KERNEL_MEMORY_PREFIX);

    // pc_screen_font_darw(test, &x, &y);

    return 0;
}
