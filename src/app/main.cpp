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
    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    gpio_function_select(16, GPFSEL_Output); // Latch
    gpio_function_select(17, GPFSEL_Output); // Clock
    gpio_function_select(18, GPFSEL_Output); // Data 1 LSB
    gpio_function_select(19, GPFSEL_Output); // Data 1 MSB

    const double samples_per_second = 8568000; 
    
    const double seconds_per_sample = 1 / samples_per_second;
    const double freqency = 1000;
    const double period = 1 / freqency;

    const double convetion_factor = (2.0f * M_PI * freqency) / samples_per_second;

    const int reqired_samples = period * samples_per_second;

    for (int i = 0; i < reqired_samples; i++)
    {
        double value = sin(convetion_factor * (double)i);

        for (int j = 1; j < 10; j++) // To make cursed waveform https://www.desmos.com/calculator/ml04pusu2o
        {
            double n = (double)j * 2.0;
            value += (1.0 / n) * sin(n * convetion_factor * (double)i);
        }

        value += 1;
        value *= 2.5; // Now is in range [0, 5]

        double output = value * 51;
        output = clamp(output, 255, 0);
        write_buffer_one_channle_8_bit_samples(((uint64_t*)0xFFFF000100000000), (uint8_t)output, i);
    }

    uart_puts("output starts now!\n");

    write_one_channel_8_bit_samples((uint64_t*)0xFFFF000100000000, ((uint64_t*)0xFFFF000100000000) + reqired_samples / sizeof(uint64_t));

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
