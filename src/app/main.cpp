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


#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main()
{
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    uint32_t height = get_framebuffer_height();
    uint32_t width = get_framebuffer_width();

    for (int y = 0 ; y < height; y++)
    {
        for (int x = 0 ; x < width; x++)
        {
            set_framebuffer_pixel(x, y, (x / (float)width) * 255, (y / (float)height) * 255, (x * y) / (float)(height * width) * 255);
        }
    }

    return 0;
}

void* operator new(size_t size)
{
    void * p = malloc(size);
    return p;
}

void operator delete(void * p)
{
    free(p);
}
