#include "io/printf.h"
#include "io/sd.h"

#include "io/propertyTags.h"
#include "lib/clocks.h"

#include "io/uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


int main()
{
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    int r = sd_init();

    if (r !=  SD_OK)
        return r;

    uint8_t buffer[512];
    r = sd_readblock(0, buffer, 1);
    
    if (r == 0)
        return -1;

    uart_put_memory_dump_formated(buffer, 512);

    return 0;
}
