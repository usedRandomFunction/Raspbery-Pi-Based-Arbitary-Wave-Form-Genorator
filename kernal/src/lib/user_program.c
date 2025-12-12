#include "lib/user_program.h"
#include "lib/config_file.h"
#include "lib/exceptions.h"
#include "io/file_access.h"
#include "lib/interrupts.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/events.h"
#include "io/framebuffer.h"
#include "io/printf.h"
#include "io/keypad.h"
#include "lib/mmu.h"

#define RETURN_VALUE_MAGIC_NUMBER 0xF55F433AE4D230F4

extern int system_call_exit_return_value;
void system_call_exit(int status); 

static user_program_info* s_active_user_program = NULL;
char* use_program_requested_switch = NULL;

// Used to load a load monolithic user program from the disk
// @param program A pointer to the user_program_info struct to fill out
// @param config A fill config file struct for the program
// @param path Path to the cfg file
static bool s_load_monolithic_user_program_from_disk(user_program_info* program, const config_file* config, const char* path);

// Uses the config file to find the image path
// @param config A filled config file struct
// @param config_path the path to the file
// @return The image path to the program
static char* s_get_program_image_path(const config_file* config, const char* config_path);

// Finds a location for a new section in the translation table, as descibed for VMEMMAP_RETURN_POINTER
// @param translation_table Translation_table for the target app
// @returns The target location for section_start as a size_t
static size_t s_vmemmap_find_pointer(translation_table_info* translation_table);

void terminate_current_user_program()
{
    if (!is_user_program_active())
        return;

    printf("\nTerminating current user program\n");

    system_call_exit(INT32_MIN);
}

void on_user_program_exiting()
{
    file_access_on_user_app_exit(); // To deal with any open files
    uart_keypad_emmulation(-2);
    capture_prg_exit(NULL);
    framebuffer_on_user_app_exit();
    keypad_polling(-2);
}

bool is_user_program_active()
{
    return s_active_user_program != NULL;
}

user_program_info* get_active_user_program()
{
    return s_active_user_program;
}

bool load_user_program_from_disk(user_program_info* program, const char* path)
{
    memclr(program, sizeof(user_program_info));

    printf("Loading user program: %s\n", path);

    if (strcmp(path + strlen(path) - 4, ".cfg")) // Check file extention
    {
        printf("Failed to laod user program: %s\n", "Unkown file extention: ");
        printf("%s\n", path + strlen(path) - 4);

        return false;
    }

    config_file config;
    if (!load_config_file(&config, path))
        return false;

    // Get program type
    const config_file_entry* program_type_entry = get_config_file_entry_from_name(&config, "APPLICATION_TYPE");

    if (program_type_entry == NULL)
    {
        printf("Failed to laod user program: %s\n", "Unable to find APPLICATION_TYPE");

        free_loaded_config_file(&config);
    }

    char program_type[11];
    if (!get_string_from_config_file_entry(program_type_entry, program_type, 11))
    {
        printf("Failed to laod user program: %s\n", "Unable to load APPLICATION_TYPE string to large");

        free_loaded_config_file(&config);
    }

    if (strcmp(program_type, "MONOLITHIC") != 0)
    {
        printf("Failed to laod user program: %s\n", "Unkown APPLICATION_TYPE: ");
        printf("%s\n", program_type);

        free_loaded_config_file(&config);
    }

    bool success = s_load_monolithic_user_program_from_disk(program, &config, path);

    free_loaded_config_file(&config);

    if (success)
        printf("Successfully loaded program %s\n", path);

    return success;
}

static char* s_get_program_image_path(const config_file* config, const char* config_path)
{
    const config_file_entry* image_entry = get_config_file_entry_from_name(config, "IMAGE_PATH");
    
    if (image_entry == NULL)
        return NULL;

    size_t config_offset = strlen(config_path);

    while (config_path[config_offset] != '/' && config_offset)
        config_offset--;

    config_offset++; // We need to keep the '/'

    size_t size = config_offset + (image_entry->value_end - image_entry->value_begin);

    char* path = malloc(size + 1);

    if (path == NULL)
        return NULL;

    memcpy(path, config_path, config_offset);
    get_string_from_config_file_entry(image_entry, path + config_offset, size - config_offset + 1);

    return path;
}

static bool s_load_monolithic_user_program_from_disk(user_program_info* program, const config_file* config, const char* path)
{
    uint64_t program_address = get_u64_from_config_file_entry_with_defult_by_name(config, "PROGRAM_ADDRESS", UINT64_MAX);

    if (program_address == UINT64_MAX)
    {
        printf("Failed to laod user program: %s\n", "Unable to find PROGRAM_ADDRESS");
        return false;
    }

    char* image_path = s_get_program_image_path(config, path);

    if (image_path == NULL)
    {
        printf("Failed to laod user program: %s\n", "Unable to find IMAGE_PATH");
        return false;
    }

    uint64_t maxium_program_size = get_u64_from_config_file_entry_with_defult_by_name(config, "MONOLITHIC_PAGE_SIZE", UINT64_MAX);
    uint64_t minium_stack_size = get_u64_from_config_file_entry_with_defult_by_name(config, "MINIUM_STACK_SIZE", 1024);
    uint64_t stack_address = get_u64_from_config_file_entry_with_defult_by_name(config, "STACK_ADDRESS", program_address + 0x000400000000);
    uint64_t program_entry = get_u64_from_config_file_entry_with_defult_by_name(config, "PROGRAM_ENTRY", program_address);
    uint64_t writability = get_u64_from_config_file_entry_with_defult_by_name(config, "PROGRAM_MEMORY_WRITABILITY", 0);
    uint64_t abi_version = get_u64_from_config_file_entry_with_defult_by_name(config, "ABI_VERSION", 0);

    if (abi_version != 0)
    {
        free(image_path);
        return false;
    }

    int image = open(image_path, 0);
    free(image_path);

    if (image == -1)
    {
        printf("Failed to laod user program: %s\n", "Unable to open image");
        return false;
    }

    size_t monolithic_page_size = maxium_program_size;
    size_t image_size = get_file_size(image);

    if (image_size == -1)
    {
        printf("Failed to laod user program: %s\n", "Unable to get image size");

        close(image);
        return false;
    }

    if (monolithic_page_size == UINT64_MAX)
        monolithic_page_size = image_size;
    
    if (!initialize_monolithic_user_program(program, (void*)program_address, 
        monolithic_page_size, (void*)program_entry, (void*)stack_address, minium_stack_size))
    {
        close(image);
        return false;
    }

    void* ttbr0 = get_ttbr0_el1();
    switch_to_addess_space_to_program(program);

    monolithic_page_size = get_page_allocation_size(program->translation_table.sections[1].allocation);
    size_t bytes_read = read(image, (void*)program_address, image_size);
    memclr((void*)(program_address + image_size), monolithic_page_size - image_size);
    memclr(program->stack_start, program->stack_end - program->stack_start);
    set_ttbr0_el1(ttbr0);

    close(image);

    if (bytes_read == -1)
    {
        printf("Failed to laod user program: %s\n", "Failed to read image");

        return false;
    }

    if (writability == 0)
        set_monolithic_user_program_writability(program, true);

    abi_version = (uint32_t)abi_version;

    return true;
}

bool initialize_monolithic_user_program(user_program_info* program, void* program_start, size_t required_size, void* entry, 
    void* stack_start, size_t required_stack)
{
    memclr(program, sizeof(user_program_info));
    program->stack_start = stack_start;
    program->entry = entry;
    
    translation_table_section_info table_sections[2];
    memclr(table_sections, sizeof(table_sections));

    table_sections[0].allocation = create_new_page_allocation(required_size);
    table_sections[0].attributes = MMU_ATTRIBUTES_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EL0_ACCESS;
    table_sections[0].section_start = program_start;

    if (required_stack != 0)
    {
        table_sections[1].allocation = create_new_page_allocation(required_stack);
        table_sections[1].attributes = MMU_ATTRIBUTES_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EXECUTE_NEVER | MMU_ATTRIBUTES_EL0_ACCESS;
        table_sections[1].section_start = stack_start;
    }

    size_t stack_size = get_page_allocation_size(table_sections[1].allocation);
    program->stack_end = void_ptr_offset_bytes(stack_start, stack_size);

    
    if (!initialize_translation_table(&(program->translation_table), table_sections, required_size == 0 ? 1 : 2))
    {
        destroy_page_allocation(table_sections[0].allocation);
        destroy_page_allocation(table_sections[1].allocation);

        printf("Failed to initialize monolithic user program\n");

        return false;
    }

    return true;
}

bool set_monolithic_user_program_writability(user_program_info* program, bool readonly)
{   
    program->translation_table.sections[0].attributes &= ~MMU_ATTRIBUTES_READ_ONLY;

    if (readonly)
        program->translation_table.sections[0].attributes |= MMU_ATTRIBUTES_READ_ONLY;
    
    return remake_translation_table_section(&(program->translation_table), 0, false);
}

void switch_to_addess_space_to_program(user_program_info* program)
{
    set_ttbr0_el1(program->translation_table.page_global_directory);
}

int execute_user_program(user_program_info* program)
{
    s_active_user_program = program;

    // We use the function here, also it exists as a function
    // As it needs to be writen in asm
    _execute_user_program(program->entry, program->stack_end);
    
    s_active_user_program = NULL;

    // Reset anything done by the user app
    on_user_program_exiting();

    return system_call_exit_return_value;
}

void destroy_user_program(user_program_info* program)
{
    destory_translation_table(&program->translation_table);
}

size_t user_program_vmemmap(user_program_info* program, void* ptr, size_t size, int flags)
{
    if (is_kernal_memory(ptr))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", ptr, "vmemmap");
        
    if (flags & VMEMMAP_RETURN_POINTER)
    {
        const size_t target = s_vmemmap_find_pointer(&program->translation_table);

        *((void**)ptr) = (void*)target;

        ptr = (void*)target;
    }

    if (((size_t)ptr) % (1 << PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO))
    {
        printf("Failed to allocate section: 0x%x is not alligned to 2^%d\n", ptr, PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO);
        return 0;
    }

    uint64_t attributes =  MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EL0_ACCESS;

        if (!(flags & VMEMMAP_EXECUTABLE))
            attributes |= MMU_ATTRIBUTES_EXECUTE_NEVER;

        if (!(flags & VMEMMAP_WRITABILITY))
            attributes |= MMU_ATTRIBUTES_READ_ONLY;

        if (flags & VMEMMAP_NON_CACHABLE)
            attributes |= MMU_ATTRIBUTES_NON_CACHABLE;
        else
            attributes |= MMU_ATTRIBUTES_CACHABLE;

    for (int i = 0; i < program->translation_table.number_of_sections; i++)
    {
        translation_table_section_info* section = program->translation_table.sections + i;

        if (section->section_start != ptr)
            continue;               // Now it only continues if we are at the target section

        if (size == 0)
        {
            return remove_translation_table_section(&program->translation_table, i, false) ? 1 : 0;
        }

        if (size == -1)
            return get_page_allocation_size(section->allocation);

        if (!resize_page_allocation(section->allocation, size))
            return 0;
        
        section->attributes = attributes;

        if (!remake_translation_table_section(&program->translation_table, i, false))
            return 0;

        return get_page_allocation_size(section->allocation);
    }

    if (size == 0)
        return 1; // We tryed to delete something that doesn't exist

    // Now its only allocations that need to be created
    
    translation_table_section_info section;

    section.section_start = ptr;
    section.attributes = attributes;
    section.allocation = create_new_page_allocation(size);

    if (section.allocation == NULL)
        return 0;

    if (insert_translation_table_section(&program->translation_table, &section, false) == false)
        return 0;

    return get_page_allocation_size(section.allocation);
}

size_t s_vmemmap_find_pointer(translation_table_info* translation_table)
{
    const size_t minimum_section_offset = 0x40000000;
    size_t new_start = minimum_section_offset;

    for (int i = 0; i < translation_table->number_of_sections; i++)
    {
        const translation_table_section_info* section = &translation_table->sections[i];
        const size_t section_start = (size_t)section->section_start;

        if (new_start < section_start)
            break;

        const size_t section_end = section_start + get_page_allocation_size(section->allocation);

        // round up
        new_start = section_end / minimum_section_offset;
        new_start += section_end % minimum_section_offset ? 1 : 0;
        new_start *= minimum_section_offset;
    }

    printf("vmemmap: found space in translation table 0x%x:\n    at: 0x%x\n", translation_table, new_start);

    return new_start;
}

void user_program_switch_to(const char* new_executable_path)
{
    size_t size = strlen(new_executable_path);

    if (use_program_requested_switch != NULL)
        free(use_program_requested_switch);

    use_program_requested_switch = malloc(size + 1);

    if (use_program_requested_switch == NULL)
        generic_user_exception("Failed to allocate space to store location of next user program.");
    
    strcpy_s(new_executable_path, size + 1, use_program_requested_switch);

    terminate_current_user_program();
}

void execute_function_as_user_program(user_program_info* program, USER_FUNCTION function)
{
    if (program != s_active_user_program)
    {
        printf("Error: execute_function_as_user_program, does not current suport switching programs\n");
        kernel_panic();
    }

    _execute_function_as_user_program(function);
}

void defult_prg_exit_handler()
{
    if (interupt_active)
    {
        printf("Waiting on interupt end before exiting program\n");
        
        event_handler_add_interupt_end(terminate_current_user_program);

        return;
    }
    
    terminate_current_user_program();
}

bool check_if_instruction_abort_is_user_program_function_return()
{
    size_t esr_el1;
    asm volatile ( "mrs %0, esr_el1" : "=r"(esr_el1));

    if ((esr_el1 >> 26) != 0b100000) // UNDONE, if it does not work try 0b100001
        return false;           // Check if it is a instruction abort from el0

    size_t far_el1;
    asm volatile ( "mrs %0, far_el1" : "=r"(far_el1));

    if (far_el1 != RETURN_VALUE_MAGIC_NUMBER)
        return false;           // Make sure its from execute_function_as_user_program

    _return_from_user_function();

    return true; 
}
