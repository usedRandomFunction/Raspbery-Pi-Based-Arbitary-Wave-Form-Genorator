#include "lib/central_block_memory_allocator.h"
#include "lib/basicMallocFunctions.h"
#include "io/propertyTags.h"
#include "app/startup.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "io/gpio.h"
#include "io/uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static void printSystemInfoAndSetMaximumclockSpeed();

static void prepareMemoryManager();


central_block_memory_allocator_header kernal_heap_allocator;


#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main()
{
    prepareMemoryManager();

    //PrintSystemSpecs();
    SetupSystemClocks(1.2f);

    gpio_function_select(17, GPFSEL_Output); // Clock
    gpio_function_select(18, GPFSEL_Output); // Data
    gpio_function_select(27, GPFSEL_Output); // Latch

    auto* set_address = get_mmio_pointer(GPSET0);
    auto* clr_address = get_mmio_pointer(GPCLR0);

    constexpr uint32_t clock_mask = 1 << 17;
    constexpr uint32_t data_mask = 1 << 18;
    constexpr uint32_t both_mask = clock_mask | data_mask;
    constexpr uint32_t mask_all = UINT32_MAX;

    while (1)
	{
        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        gpio_write(27, 1);
        gpio_write(27, 0);
	}

    return 0;
}


static void prepareMemoryManager()
{
    size_t allocator_space = PROGRAM_END_ADDRESS_SIZE_T;
    size_t mannagedSpace = 2 * 1024 * 1024; // As this is the smallest page size we will use all of it

    if (allocator_space & 0x1FFFFF != 0) // If the address is not 2 Mib alligned
    {
        allocator_space >>= 21;
        allocator_space++;
        allocator_space <<= 21;         // Bitwise round up
    }

    initialize_central_block_memory_allocator(*(void**)&allocator_space, mannagedSpace, 5, &kernal_heap_allocator);
}

void free(void* p)
{
    central_block_memory_allocator_free(p, &kernal_heap_allocator);
}

void* malloc(size_t size)
{
    return central_block_memory_allocator_alloc(size, &kernal_heap_allocator);
}

void* aligned_alloc(size_t alignment, size_t size)
{
    if (alignment != 0)
    {
        if (((alignment & (alignment - 1)) != 0))
        {
            uart_puts("\nUnable to allocat memory, only alight ments of a power of two");
            return nullptr;
        }

        for (int i = 0; i < sizeof(size_t)*8; i++)
        {
            if (alignment & 0b1)
            {
                alignment = i;
                break;
            }

            alignment >>= 1;
        }
    }
    return central_block_memory_allocator_alloc_alligned(size, alignment, &kernal_heap_allocator);
}