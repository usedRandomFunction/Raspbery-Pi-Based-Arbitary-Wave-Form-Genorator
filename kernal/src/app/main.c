#include "lib/user_program.h"
#include "lib/config_file.h"
#include "io/file_access.h"
#include "lib/memory.h"
#include "io/printf.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static void s_print_cfg_entry(const char* name, config_file* cfg)
{
    const config_file_entry* entry = get_config_file_entry_from_name(cfg, name);

    if (entry == NULL)
    {
        printf("%s: NOT PRESSENT\n", name);
        return;
    }

    char buf[1024];
    get_string_from_config_file_entry(entry, buf, 1024);

    printf("%s = %s\n", name , buf);
}

static void s_print_cfg_entry_as_number(const char* name, config_file* cfg)
{
    const config_file_entry* entry = get_config_file_entry_from_name(cfg, name);

    if (entry == NULL)
    {
        printf("%s: NOT PRESSENT\n", name);
        return;
    }

    uint64_t n = get_u64_from_config_file_entry(entry, NULL);

    printf("%s = 0x%x\n", name , n);
}

int main()
{
    config_file cfg;

    if (load_config_file(&cfg, "hworld.cfg") == false)
        return -1;


    s_print_cfg_entry("APPLICATION_TYPE", &cfg);
    s_print_cfg_entry_as_number("MONOLITHIC_PAGE_SIZE", &cfg);
    s_print_cfg_entry_as_number("MINIUM_STACK_SIZE", &cfg);
    s_print_cfg_entry_as_number("PROGRAM_ADDRESS", &cfg);
    s_print_cfg_entry_as_number("PROGRAM_ENTRY", &cfg);
    s_print_cfg_entry_as_number("ABI_VERSION", &cfg);
    s_print_cfg_entry("IMAGE_PATH", &cfg);

    free_loaded_config_file(&cfg);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    // user_program_info program;

    // int file = open("hworld.img", 0);

    // if (file == -1)
    // {
    //     printf("Failed to load user program\n");
    //     return -1;
    // }

    // size_t progrma_size = get_file_size(file);

    // if (progrma_size == -1)
    //     return -1;

    // if (initialize_monolithic_user_program(&program, (void*)0x000040000000, progrma_size, (void*)0x000040000000, 1024) == false)
    // {
    //     close(file);
    //     return -1;
    // }

    // switch_to_addess_space_to_program(&program);

    // size_t bytes_read = read(file, (void*)0x000040000000, progrma_size);

    // close(file);
    
    // if (bytes_read == -1)
    // {
    //     printf("Failed to load user program");
    //     destroy_user_program(&program);
    //     return - 1;
    // }

    // printf("User program exicuted and returned: %d\n", execute_user_program(&program));
    // destroy_user_program(&program);

    return 0;
}
