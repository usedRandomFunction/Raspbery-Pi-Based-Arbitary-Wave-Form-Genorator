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

static void prepareMemoryManager();

#include "lib/mmu.h" // TODO move this functionality to a diffrent file

central_block_memory_allocator_header kernal_heap_allocator;


#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main()
{
    prepareMemoryManager();

    SetupSystemClocks(1.2f);

    uint64_t* mmu_pgd = (uint64_t*)aligned_alloc(4096, 8 + 8);
    uint64_t* mmu_pud = (uint64_t*)aligned_alloc(4096, 8 + 8);
    uint64_t* mmu_pmd = (uint64_t*)aligned_alloc(4096, 8 * 2 + 8);

    constexpr size_t mmu_ptr_mask = 0x1FFFFF;


    write_page_descriptor(mmu_pgd, void_ptr_bitwise_and(mmu_pud, mmu_ptr_mask), 0x0, 0x0, true);
    mmu_pgd[1] = 0; // Setting this to zero so the mmu knows its not an entry
    write_page_descriptor(mmu_pud, void_ptr_bitwise_and(mmu_pmd, mmu_ptr_mask), 0x0, 0x0, true);
    mmu_pud[1] = 0;
    
    void* mapping_ptr = void_ptr_bitwise_and(PROGRAM_START_ADDRESS_POINTER, mmu_ptr_mask);

    write_page_descriptor(mmu_pmd, mapping_ptr, 0x0, 0b1 << 8 | 0b1, false);
    write_page_descriptor(mmu_pmd + 1, void_ptr_offset_bytes(mapping_ptr, 2097152), // 2 MiB Offset
        0x0, 0b1 << 8 | 0b1, false); // Flags to be re written latter
    mmu_pmd[3] = 0;

    size_t ttbr_value = *((size_t*)&mmu_pgd) & mmu_ptr_mask;

    asm volatile ("msr ttbr1_el1, x0"
	:
	: "r" (ttbr_value)
	: "x0");

    uart_puts("MMU is controlled by c!");

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