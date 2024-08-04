#include "app/data_output.h"
#include "io/propertyTags.h"
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

// // Tempoary for testing
// #include "lib/translation_table.h"
// #include "lib/page_allocator.h"
// #include "lib/mmu.h"


#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main()
{
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    return 0;
}

#ifndef __INTELLISENSE__ // Vscode hate this line for some unknow reason
void* operator new(size_t size)
{
    void * p = malloc(size);
    return p;
}
#endif

void operator delete(void * p)
{
    free(p);
}
