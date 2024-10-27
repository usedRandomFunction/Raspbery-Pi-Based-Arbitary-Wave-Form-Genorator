#include "lib/user_program.h"
#include "lib/config_file.h"
#include "io/file_access.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "io/printf.h"
#include "lib/mmu.h"

extern int SYSTEM_CALL_EXIT_RETURN_VALUE;

// Used to load a load monolithic user program from the disk
// @param program A pointer to the user_program_info struct to fill out
// @param config A fill config file struct for the program
// @param path Path to the cfg file
static bool s_load_monolithic_user_program_from_disk(user_program_info* program, const config_file* config, const char* path);

bool load_user_program_from_disk(user_program_info* program, const char* path)
{
    memclr(program, sizeof(user_program_info));

    printf("Loading user program: %s\n", path);

    if (strcmp(path + strlen(path) - 4, ".cfg")) // Check file extention
    {
        printf("Failed to laod user program: %s\n", "Unkown file extention: ");
        printf("%s\n", path + strlen(path) - 4);
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

// Uses the config file to find the image path
// @param config A filled config file struct
// @param config_path the path to the file
// @return
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
    uint64_t program_entry = get_u64_from_config_file_entry_with_defult_by_name(config, "PROGRAM_ENTRY", program_address);
    uint64_t writability = get_u64_from_config_file_entry_with_defult_by_name(config, "PROGRAM_MEMORY_WRITABILITY", 0);
    uint64_t abi_version = get_u64_from_config_file_entry_with_defult_by_name(config, "ABI_VERSION", 0);

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
        monolithic_page_size, (void*)program_entry, minium_stack_size))
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

    if (required_size != 0)
    {
        table_sections[1].allocation = create_new_page_allocation(required_stack);
        table_sections[1].attributes = MMU_ATTRIBUTES_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EXECUTE_NEVER | MMU_ATTRIBUTES_EL0_ACCESS;
        table_sections[1].section_start = program_stack;
    }

    size_t stack_size = get_page_allocation_size(table_sections[1].allocation);
    program->stack_end = void_ptr_offset_bytes(program_stack, stack_size);

    
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
    // We use the function here, also it exists as a function
    // As it needs to be writen in asm
    user_program_internal_use_program_excute(program->entry, program->stack_end);
    
    return SYSTEM_CALL_EXIT_RETURN_VALUE;
}

void destroy_user_program(user_program_info* program)
{
    destory_translation_table(&program->translation_table);
}