#include "io/propertyTags.h"
#include "io/framebuffer.h"
#include "app/startup.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/math.h"
#include "io/gpio.h"
#include "io/uart.h"

#include "lib/timing.h"
#include "io/printf.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


int main()
{
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    // printf("hello world, %x\n", KERNEL_MEMORY_PREFIX);
    for (int i = 0; i < 100; i++)
    {
        printf("hello world, %d\n", i);
        delay_milliseconds(100);
    }

    return 0;
}
