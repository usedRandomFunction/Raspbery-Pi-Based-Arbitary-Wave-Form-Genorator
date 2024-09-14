#include "lib/central_block_memory_allocator.h"
#include "lib/translation_table.h"
#include "boot/boardDectetion.h"
#include "lib/arm_exceptions.h"
#include "lib/page_allocator.h"
#include "io/memoryMappedIO.h"
#include "io/pc_screen_font.h"
#include "io/framebuffer.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "io/putchar.h"
#include "io/printf.h"
#include "lib/mmu.h"
#include "io/uart.h"
#include "io/sd.h"

#include <stdbool.h>
#include <stdint.h>

extern pc_screen_font_header _binary_data_font_psf_start;
central_block_memory_allocator_header kernel_heap_allocator;
translation_table_info kernel_translation_table;

static void s_initialize_virtual_address_translation();
static void s_prepare_memory_manager();
static void s_init_defult_values();

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
    s_init_defult_values();

	#ifdef UART_BAUD_RATE
	uart_init(UART_BAUD_RATE);
	#else
	uart_init(115200);
	#endif

	printf("Started system, board type: %s\n", get_board_name(boardType));

	s_prepare_memory_manager();
    s_initialize_virtual_address_translation(); // Must be called before *ANY* calls to malloc are made

    if (!initialize_framebuffer(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT))
    {
        kernel_panic();
    }

    if (initialize_sd() !=  SD_OK)
    {
        printf("Initialize SD failed!\n");
        kernel_panic();
    }
    
	printf("Starting main function!\n");
	int result = main();

	printf("Program ended with code: %d!\n", result);

	while (1)
    {
        uart_getc();
        printf("System halted please restart!");
    }
}

static void s_init_defult_values()
{
    uart_initall_init_has_occured = false;
    is_frambuffer_initialized = false;
    current_font = &_binary_data_font_psf_start;
    putchar_init_values();
}

static void s_prepare_memory_manager()
{
    size_t allocator_space = PROGRAM_END_ADDRESS_SIZE_T;
    size_t mannagedSpace = 2 * 1024 * 1024; // As this is the smallest page size we will use all of it
    if ((allocator_space & 0x1FFFFF) != 0) // If the address is not 2 Mib alligned
    {
        allocator_space >>= 21;
        allocator_space++;
        allocator_space <<= 21;         // Bitwise round up
    }
    initialize_central_block_memory_allocator((void*)allocator_space, mannagedSpace, 5, &kernel_heap_allocator);
}

static void s_initialize_virtual_address_translation() 
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

    printf("Enabled temporary transliation table\n");
    
    void* new_allocator_base_address = (void*) 0xFFFF000040000000;
    ptrdiff_t new_heap_location_offset = new_allocator_base_address - kernel_heap_allocator.controll_region_start;
    kernel_heap_allocator.controll_region_start = new_allocator_base_address;
    kernel_heap_allocator.allocation_region_start += new_heap_location_offset;
    temporary_kernel_heap_pmd = (uint64_t*)void_ptr_offset_bytes(temporary_kernel_heap_pmd, new_heap_location_offset);
    temporary_kernel_code_pmd = (uint64_t*)void_ptr_offset_bytes(temporary_kernel_code_pmd, new_heap_location_offset);
    temporary_pud = (uint64_t*)void_ptr_offset_bytes(temporary_pud, new_heap_location_offset);
    temporary_pgd = (uint64_t*)void_ptr_offset_bytes(temporary_pgd, new_heap_location_offset);

    printf("Remapped memory allocator to virutal address 0xFFFF000040000000\n");

    if (!initialize_page_allocator())
        kernel_panic();

    ptrdiff_t allocation_offset;
    
    page_allocation_info* kernel_code_page_allocation = create_new_page_allocation_at_continuous_physical_address(mapping_ptr, kernel_code_size, 
        &allocation_offset);

    if (kernel_code_page_allocation == NULL)
        kernel_panic();

    if (allocation_offset != 0)
    {
        printf("Failed to create kernel code allocation: offset != 0\n");
        kernel_panic();
    }

    page_allocation_info* kernel_heap_page_allocation = create_new_page_allocation_at_continuous_physical_address(
        void_ptr_offset_bytes(mapping_ptr, kernel_minium_sections << 21), 1 << 21, &allocation_offset);

    if (allocation_offset != 0)
    {
        printf("Failed to create kernel heap allocation: offset != 0\n");
        kernel_panic();
    }

    if (kernel_heap_page_allocation == NULL)
        kernel_panic();

    printf("Create kernel code and kernel heap, page allocations\n");

    translation_table_section_info table_sections[3];
    memclr(table_sections, sizeof(table_sections));

    table_sections[0].allocation = kernel_code_page_allocation;
    table_sections[0].lowwer_attributes = MMU_LOWER_ATTRIBUTES_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_sections[0].section_start = (void*)0x000000000000; // We dont include the FFFF prefix here
    table_sections[1].allocation = kernel_heap_page_allocation;
    table_sections[1].lowwer_attributes = MMU_LOWER_ATTRIBUTES_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_sections[1].upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_sections[1].section_start = (void*)0x000040000000; // We dont include the FFFF prefix here

    // MMIO section
    ptrdiff_t mmio_allocation_offset;
    page_allocation_info* mmio_allocation = create_new_page_allocation_for_unmanaged_continuous_physical_address((void*)MMIO_Base_Address, 1 << 24, 
        &mmio_allocation_offset);
    table_sections[2].allocation = mmio_allocation;
    table_sections[2].lowwer_attributes = MMU_LOWER_ATTRIBUTES_nGnRnE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_sections[2].upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_sections[2].section_start = MMIO_VIRUTAL_ADDRESS_BASE;

    if (!initialize_translation_table(&kernel_translation_table, table_sections, sizeof(table_sections) / sizeof(table_sections[0])))
        kernel_panic();
    
    set_ttbr1_el1(get_physical_address(kernel_translation_table.page_global_directory));

    printf("Switched to permante translation_table.\n");
    free(temporary_kernel_code_pmd);
    free(temporary_kernel_heap_pmd);
    free(temporary_pud);
    free(temporary_pgd);

    MMIO_Base_Address = ((size_t)MMIO_VIRUTAL_ADDRESS_BASE) + 0xFFFF000000000000 + mmio_allocation_offset;
    set_ttbr0_el1(NULL); // Since we arn't using 0x0000000000000000 to 0x0000FFFFFFFFFFFF anymore we should un map it

    printf("Remapped MMIO to %x\n", MMIO_Base_Address);
}

void free(void* p)
{
    if (p == NULL)
    {
        printf("kernel attempted to free NULL pointer");
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
            printf("\nUnable to allocat memory, only alight ments of a power of two");
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