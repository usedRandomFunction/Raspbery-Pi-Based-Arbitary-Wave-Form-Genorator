#include "lib/central_block_memory_allocator.h"
#include "lib/translation_table.h"
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

central_block_memory_allocator_header kernel_non_cachable_heap_allocator;
central_block_memory_allocator_header kernel_heap_allocator;
translation_table_info kernel_translation_table;

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
    uint64_t* temporary_pgd = (uint64_t*)aligned_alloc(4096, 8);
    uint64_t* temporary_pud = (uint64_t*)aligned_alloc(4096, 8 * 2);

    size_t kernel_code_size = PROGRAM_END_ADDRESS_SIZE_T - PROGRAM_START_ADDRESS_SIZE_T;
    size_t kernel_minium_sections = (kernel_code_size >> 21) + ((kernel_code_size & ((1 << 21) - 1)) ? 1 : 0);

    uint64_t* temporary_kernel_code_pmd = (uint64_t*)aligned_alloc(4096, 8 * kernel_minium_sections);
    uint64_t* temporary_kernel_heap_pmd = (uint64_t*)aligned_alloc(4096, 8 );
    
    void* mapping_ptr = get_physical_address(PROGRAM_START_ADDRESS_POINTER);
    write_page_descriptor(temporary_pgd, get_physical_address(temporary_pud), 0x0, 0x0, true);
    write_page_descriptor(temporary_pud, get_physical_address(temporary_kernel_code_pmd), 0x0, 0x0, true);
    write_page_descriptor(temporary_pud + 1, get_physical_address(temporary_kernel_heap_pmd), 0x0, 0x0, true);

    // Both of these are being (temporay set) as non-cache able memory
    for (int i = 0; i < (int)kernel_minium_sections; i++)
        write_page_descriptor(temporary_kernel_code_pmd + i, void_ptr_offset_bytes(mapping_ptr, i << 21), 0x0, 
        MMU_LOWER_ATTRIBUTES_NON_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT, false);

    write_page_descriptor(temporary_kernel_heap_pmd, void_ptr_offset_bytes(mapping_ptr, kernel_minium_sections << 21), 
    MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER, MMU_LOWER_ATTRIBUTES_NON_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT, false);

    set_ttbr1_el1(get_physical_address(temporary_pgd));

    uart_puts("Enabled temporary transliation table\n");
    
    void* new_allocator_base_address = (void*) 0xFFFF000040000000;
    ptrdiff_t new_heap_location_offset = new_allocator_base_address - kernel_heap_allocator.controll_region_start;
    kernel_heap_allocator.controll_region_start = new_allocator_base_address;
    kernel_heap_allocator.allocation_region_start += new_heap_location_offset;
    temporary_kernel_heap_pmd = (uint64_t*)void_ptr_offset_bytes(temporary_kernel_heap_pmd, new_heap_location_offset);
    temporary_kernel_code_pmd = (uint64_t*)void_ptr_offset_bytes(temporary_kernel_code_pmd, new_heap_location_offset);
    temporary_pud = (uint64_t*)void_ptr_offset_bytes(temporary_pud, new_heap_location_offset);
    temporary_pgd = (uint64_t*)void_ptr_offset_bytes(temporary_pgd, new_heap_location_offset);

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

    translation_table_section_info table_sections[4];
    memclr(table_sections, sizeof(table_sections));

    table_sections[0].allocation = kernel_code_page_allocation;
    table_sections[0].lowwer_attributes = MMU_LOWER_ATTRIBUTES_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_sections[0].section_start = (void*)0x000000000000; // We dont include the FFFF prefix here
    table_sections[1].allocation = kernel_heap_page_allocation;
    table_sections[1].lowwer_attributes = MMU_LOWER_ATTRIBUTES_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_sections[1].upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_sections[1].section_start = (void*)0x000040000000; // We dont include the FFFF prefix here

    // MMIO section
    // This looks bad becouse it is lamo
    // Since the mmio memory wont be allocateable we have to fake it
    page_allocation_info* mmio_allocation = malloc(sizeof(page_allocation_info));
    mmio_allocation->first_page = (MMIO_Base_Address / page_allocator_page_size_bytes);
    mmio_allocation->size = (1 << 24) / page_allocator_page_size_bytes; // Just assume the MMIO is 2^24 bytes
    mmio_allocation->next = NULL;
    table_sections[2].allocation = mmio_allocation;
    table_sections[2].lowwer_attributes = MMU_LOWER_ATTRIBUTES_nGnRnE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_sections[2].upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_sections[2].section_start = (void*)0x000080000000; // We dont include the FFFF prefix here

    page_allocation_info* non_cachable_heap_allocation = create_new_page_allocation(1024 * 16); 
    // We create a heap of size 16 Kib, ik that at the moment the page size 2 Mib but the small size is for latter,
    // When the page size is smaller
    table_sections[3].allocation = non_cachable_heap_allocation;
    table_sections[3].lowwer_attributes = MMU_LOWER_ATTRIBUTES_NON_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_sections[3].upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_sections[3].section_start = (void*)0x0000C0000000; // We dont include the FFFF prefix here

    if (!initialize_translation_table(&kernel_translation_table, table_sections, sizeof(table_sections) / sizeof(table_sections[0])))
        kernel_panic();
    
    set_ttbr1_el1(get_physical_address(kernel_translation_table.page_global_directory));

    uart_puts("Switched to permante translation_table.\n");
    free(temporary_kernel_code_pmd);
    free(temporary_kernel_heap_pmd);
    free(temporary_pud);
    free(temporary_pgd);

    MMIO_Base_Address = 0xFFFF000080000000;
    set_ttbr0_el1(NULL); // Since we arn't using 0x0000000000000000 to 0x0000FFFFFFFFFFFF anymore we should un map it

    uart_puts("Remapped MMIO to 0xFFFF000080000000.\n");

    initialize_central_block_memory_allocator((void*)0xFFFF0000C0000000, 1024 * 16, 5, &kernel_non_cachable_heap_allocator);

}

void free(void* p)
{
    if (p == NULL)
    {
        uart_puts("kernel attempted to free NULL pointer");
        kernel_panic();
    }

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

void free_noncachable_memory(void* p)
{
    if (p == NULL)
    {
        uart_puts("kernel attempted to free NULL pointer");
        kernel_panic();
    }

    central_block_memory_allocator_free(p, &kernel_non_cachable_heap_allocator);
}

void* aligned_alloc_noncachable(size_t alignment, size_t size)
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
    
    return central_block_memory_allocator_alloc_alligned(size, alignment, &kernel_non_cachable_heap_allocator);
}
