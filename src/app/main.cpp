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


central_block_memory_allocator_header memoryAllocator;

#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main()
{	
    prepareMemoryManager();

    PrintSystemSpecs();
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


#define managedMemoryRegionSizeKib 256

static void prepareMemoryManager()
{
	// size_t end_address = PROGRAM_END_ADDRESS_SIZE_T;
    // size_t mannagedSpace = managedMemoryRegionSizeKib * 1024 - end_address;

    size_t mannagedSpace = managedMemoryRegionSizeKib * 1024;
    initialize_central_block_memory_allocator(nullptr, mannagedSpace, 5, &memoryAllocator);
}

void free(void* p)
{
    central_block_memory_allocator_free(p, &memoryAllocator);
}

void* malloc(size_t size)
{
    return central_block_memory_allocator_alloc(size, &memoryAllocator);
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
    return central_block_memory_allocator_alloc_alligned(size, alignment, &memoryAllocator);
}