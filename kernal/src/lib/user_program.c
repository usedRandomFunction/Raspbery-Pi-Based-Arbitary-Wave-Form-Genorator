#include "lib/user_program.h"
#include "lib/memory.h"
#include "lib/mmu.h"
#include "io/printf.h"

extern int SYSTEM_CALL_EXIT_RETURN_VALUE;



bool initialize_monolithic_user_program(user_program_info* program, void* program_start, size_t required_size, void* entry, size_t required_stack)
{
    void * program_stack = void_ptr_offset_bytes(program_start, 0x000400000000); // We'll just place the stack 128 GiB above the program
    
    memclr(program, sizeof(user_program_info));
    program->stack_start = program_stack;
    program->entry = entry;
    
    translation_table_section_info table_sections[2];
    memclr(table_sections, sizeof(table_sections));

    table_sections[0].allocation = create_new_page_allocation(required_size);
    table_sections[0].attributes = MMU_ATTRIBUTES_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EL0_ACCESS;
    table_sections[0].section_start = program_start;

    table_sections[1].allocation = create_new_page_allocation(required_stack);
    table_sections[1].attributes = MMU_ATTRIBUTES_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EXECUTE_NEVER | MMU_ATTRIBUTES_EL0_ACCESS;
    table_sections[1].section_start = program_stack;

    size_t stack_size = get_page_allocation_size(table_sections[1].allocation);
    program->stack_end = void_ptr_offset_bytes(program_start, stack_size);

    
    if (!initialize_translation_table(&(program->table), table_sections, sizeof(table_sections) / sizeof(table_sections[0])))
    {
        destroy_page_allocation(table_sections[0].allocation);
        destroy_page_allocation(table_sections[1].allocation);

        printf("Failed to initialize monolithic user program\n");

        return false;
    }

    return true;
}



bool toggle_monolithic_user_program_writability(user_program_info* program, bool readonly)
{   
    program->table.sections[0].attributes &= ~MMU_ATTRIBUTES_READ_ONLY;

    if (readonly)
        program->table.sections[0].attributes |= MMU_ATTRIBUTES_READ_ONLY;
    
    return remake_translation_table_section(&(program->table), 0, false);
}

void switch_to_addess_space_to_program(user_program_info* program)
{
    set_ttbr0_el1(program->table.page_global_directory);
}

int execute_user_program(user_program_info* program)
{
    // We use the function here, also it exists as a function
    // As it needs to be writen in asm
    user_program_internal_use_program_excute(program->entry, program->stack_end);
    
    return SYSTEM_CALL_EXIT_RETURN_VALUE;
}

void destroy_user_program(user_program_info* program)
{
    destory_translation_table(&program->table);
}