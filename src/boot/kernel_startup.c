#include "lib/central_block_memory_allocator.h"
#include "boot/boardDectetion.h"
#include "lib/arm_exceptions.h"
#include "lib/page_allocator.h"
#include "io/memoryMappedIO.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/mmu.h"
#include "io/uart.h"

#include <stdbool.h>
#include <stdint.h>

central_block_memory_allocator_header kernel_heap_allocator;

static void initialize_virtual_address_translation();
static void prepare_memory_manager();

int main(); // The main system Function
 
#ifdef AARCH64
// arguments for AArch64
void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
	// Declare as unused
	// (void) dtb_ptr32;
	// (void) x1;
	// (void) x2;
	// (void) x3;
#else
// arguments for AArch32
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
	// Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;
#endif
{
	int boardType = get_board_type();
	set_mmio_base(boardType);

	#ifdef UART_BAUD_RATE
	uart_init(UART_BAUD_RATE);
	#else
	uart_init(115200);
	#endif

	uart_puts("Started system, board type: ");
	uart_puts(get_board_name(boardType));

	uart_puts("\nException level: ");
    unsigned int reg = 0;
    asm volatile ("mrs %x0, CurrentEL" : "=r" (reg));
    uart_putui(reg >> 2);
    uart_putc('\n');


	prepare_memory_manager();
    initialize_virtual_address_translation(); // Must be called before *ANY* calls to malloc are made

	uart_puts("Starting main function!\n");
	int result = main();

	uart_puts("Program ended with code: ");
	uart_puti(result);
	uart_puts("!\n");
	while (1)
		uart_putc(uart_getc());
}

static void prepare_memory_manager()
{
    size_t allocator_space = PROGRAM_END_ADDRESS_SIZE_T;
    size_t mannagedSpace = 2 * 1024 * 1024; // As this is the smallest page size we will use all of it
    if ((allocator_space & 0x1FFFFF) != 0) // If the address is not 2 Mib alligned
    {
        allocator_space >>= 21;
        allocator_space++;
        allocator_space <<= 21;         // Bitwise round up
    }
    initialize_central_block_memory_allocator(*(void**)&allocator_space, mannagedSpace, 5, &kernel_heap_allocator);
}

static void initialize_virtual_address_translation() 
{
    // Since the memory allocator is suposted to live at 0xFFFF000040000000 and the tables live in the heap we need to 
    // tempoaryly set up that address in the mmu
    uint64_t* temporary_pgd = (uint64_t*)aligned_alloc(4096, 8 + 8); // TODO it may be possible to just have one pgd and overwrite
    uint64_t* temporary_pud = (uint64_t*)aligned_alloc(4096, 8 * 2 + 8);

    size_t kernel_code_size = PROGRAM_END_ADDRESS_SIZE_T - PROGRAM_START_ADDRESS_SIZE_T;
    size_t kernel_minium_sections = (kernel_code_size >> 21) + ((kernel_code_size & ((1 << 21) - 1)) ? 1 : 0);

    uint64_t* temporary_kernel_code_pmd = (uint64_t*)aligned_alloc(4096, 8 * kernel_minium_sections + 8);
    uint64_t* temporary_kernel_heap_pmd = (uint64_t*)aligned_alloc(4096, 8 + 8);
    
    void* mapping_ptr = get_physical_address(PROGRAM_START_ADDRESS_POINTER);
    write_page_descriptor(temporary_pgd, get_physical_address(temporary_pud), 0x0, 0x0, true);
    temporary_pgd[1] = 0; // Setting this to zero so the mmu knows its not an entry
    write_page_descriptor(temporary_pud, get_physical_address(temporary_kernel_code_pmd), 0x0, 0x0, true);
    write_page_descriptor(temporary_pud + 1, get_physical_address(temporary_kernel_heap_pmd), 0x0, 0x0, true);
    temporary_pud[2] = 0;

    // Both of these are being (temporay set) as non-cache able memory
    for (int i = 0; i < (int)kernel_minium_sections; i++)
        write_page_descriptor(temporary_kernel_code_pmd + i, void_ptr_offset_bytes(mapping_ptr, i << 21), 0x0, 0b1 << 8 | 0b1, false);
    temporary_kernel_code_pmd[kernel_minium_sections] = 0;

    write_page_descriptor(temporary_kernel_heap_pmd, void_ptr_offset_bytes(mapping_ptr, kernel_minium_sections << 21),
        0x0, 0b1 << 8 | 0b1, false);
    temporary_kernel_heap_pmd[1] = 0;

    set_ttbr1_el1(get_physical_address(temporary_pgd));

    uart_puts("Enabled temporary transliation table\n");
    
    void* new_allocator_base_address = (void*) 0xFFFF000040000000;
    ptrdiff_t offset = new_allocator_base_address - kernel_heap_allocator.controll_region_start;
    kernel_heap_allocator.controll_region_start = new_allocator_base_address;
    kernel_heap_allocator.allocation_region_start += offset;
    temporary_kernel_heap_pmd += offset;
    temporary_kernel_code_pmd += offset;
    temporary_pud += offset;
    temporary_pgd += offset;

    uart_puts("Remapped memory allocator to virutal address 0xFFFF000040000000\n");

    if (!initialize_page_allocator())
        kernel_panic();
    
    page_allocation_info* kernel_code_page_allocation = create_new_page_allocation_at_continuous_physical_address(mapping_ptr, kernel_code_size);

    if (kernel_code_page_allocation == NULL)
        kernel_panic();

    page_allocation_info* kernel_heap_page_allocation = create_new_page_allocation_at_continuous_physical_address(void_ptr_offset_bytes(mapping_ptr, kernel_minium_sections << 21), 1 << 21);

    if (kernel_heap_page_allocation == NULL)
        kernel_panic();

    uart_puts("Create kernel code and kernel heap, page allocations\n");
}

void free(void* p)
{
    central_block_memory_allocator_free(p, &kernel_heap_allocator);
}

void* malloc(size_t size)
{
    return central_block_memory_allocator_alloc(size, &kernel_heap_allocator);
}

void* aligned_alloc(size_t alignment, size_t size)
{
    if (alignment != 0)
    {
        if (((alignment & (alignment - 1)) != 0))
        {
            uart_puts("\nUnable to allocat memory, only alight ments of a power of two");
            return NULL;
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
    return central_block_memory_allocator_alloc_alligned(size, alignment, &kernel_heap_allocator);
}