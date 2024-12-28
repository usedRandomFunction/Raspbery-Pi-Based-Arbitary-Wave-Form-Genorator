#include "run_time_kernal_config.h"

#include "lib/config_file.h"
#include "lib/alloc.h"
#include "io/printf.h"

#include <stddef.h>

char* main_interface_app_path = NULL;
bool allow_physical_keypad;
int physical_keypad_default_delay;
bool allow_uart_keypad_emmulation;
bool uart_keypad_emmulation_default_state;
uint32_t spi_clock_frequency;
int prg_exit_debounce_time;

bool is_running_in_qemu = false;


bool load_kernal_configuration()
{
    config_file config;

    if (!load_config_file(&config, "config/system.cfg"))
        return false;
    
    const config_file_entry* working_entry = get_config_file_entry_from_name(&config, "MAIN_INTERFACE_APP");

    main_interface_app_path = get_string_from_config_file_entry_allocated(working_entry);

    allow_physical_keypad = get_u64_from_config_file_entry_with_defult_by_name(&config, "ALLOW_PHYSICAL_KEYPAD", 1) > 0;
    physical_keypad_default_delay = (int)get_u64_from_config_file_entry_with_defult_by_name(&config, "PHYSICAL_KEYPAD_DEFAULT_DELAY", 50);
    allow_uart_keypad_emmulation = get_u64_from_config_file_entry_with_defult_by_name(&config, "ALLOW_UART_KEYPAD_EMMULATION", 0) > 0;
    uart_keypad_emmulation_default_state = get_u64_from_config_file_entry_with_defult_by_name(&config, "UART_KEYPAD_EMMULATION_DEFAULT_STATE", 0) > 0;
    spi_clock_frequency = get_u64_from_config_file_entry_with_defult_by_name(&config, "SPI_CLOCK_FREQUENCY", 30000000);
    prg_exit_debounce_time = get_u64_from_config_file_entry_with_defult_by_name(&config, "PRG_EXIT_DEBOUNCE_TIME", 200);

    is_running_in_qemu = get_u64_from_config_file_entry_with_defult_by_name(&config, "IS_RUNNING_IN_QEMU", 0) != 0;

    free_loaded_config_file(&config);
    printf("Successfuly loaded kernal configuration\n");

    return true;
}

void free_kernal_configuration()
{
    if (main_interface_app_path)
        free(main_interface_app_path);
}