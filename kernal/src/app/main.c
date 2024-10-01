#include "lib/translation_table.h"
#include "io/file_access.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "io/printf.h"
#include "lib/mmu.h"
#include "io/sd.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern void test_el0(void* user_memory);

int main()
{
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    translation_table_info user_table;

    translation_table_section_info table_sections[2];
    memclr(table_sections, sizeof(table_sections));
    void* user_memory = (void*)0x000040000000;
    uint8_t* user_stack = (void*)0x000080000000;
    size_t stack_size = 4096 * 2;

    table_sections[0].allocation = create_new_page_allocation(1 << PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO);
    table_sections[0].attributes = MMU_ATTRIBUTES_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EL0_ACCESS;
    table_sections[0].section_start = user_memory; // We dont include the FFFF prefix here
    table_sections[1].allocation = create_new_page_allocation(stack_size);
    table_sections[1].attributes = MMU_ATTRIBUTES_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EXECUTE_NEVER | MMU_ATTRIBUTES_EL0_ACCESS;
    table_sections[1].section_start = user_stack; // We dont include the FFFF prefix here

    if (!initialize_translation_table(&user_table, table_sections, sizeof(table_sections) / sizeof(table_sections[0])))
    {
        destroy_page_allocation(table_sections[0].allocation);
        return -1;
    }

    set_ttbr0_el1(user_table.page_global_directory);
    
    int file = open("hworld.img", 0);

    printf("FD = %d\n", file);
    if (file == -1)
    {
        destory_translation_table(&user_table);
        return -1;
    }

    printf("Reading program!\n");
    size_t return_ = read(file, user_memory, 1 << PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO);
    close(file);

    if (return_ == -1)
    {
        destory_translation_table(&user_table);
        return -1;
    }

    printf("Attempting to execute!\n");

    asm volatile ("msr sp_el0, %0"
	:
	: "r" (user_stack + stack_size));

    test_el0(user_memory);

    destory_translation_table(&user_table);

    return 0;
}
