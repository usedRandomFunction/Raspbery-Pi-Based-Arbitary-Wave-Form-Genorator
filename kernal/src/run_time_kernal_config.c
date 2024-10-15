#include "run_time_kernal_config.h"

#include "lib/config_file.h"
#include "lib/alloc.h"
#include "io/printf.h"

#include <stdint.h>
#include <stddef.h>

char* main_interface_app_path = NULL;

bool load_kernal_configuration()
{
    config_file config;

    if (!load_config_file(&config, "system.cfg"))
        return false;
    
    const config_file_entry* working_entry = get_config_file_entry_from_name(&config, "MAIN_INTERFACE_APP");

    main_interface_app_path = get_string_from_config_file_entry_allocated(working_entry);



    free_loaded_config_file(&config);
    printf("Successfuly loaded kernal configuration\n");

    return true;
}

void free_kernal_configuration()
{
    if (main_interface_app_path)
        free(main_interface_app_path);
}