#ifndef RUN_TIME_KERNAL_CONFIG_H
#define RUN_TIME_KERNAL_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

extern char* main_interface_app_path;
extern bool allow_physical_keypad;
extern int physical_keypad_default_delay;
extern bool allow_uart_keypad_emmulation;
extern bool uart_keypad_emmulation_default_state;
extern uint32_t spi_clock_frequency;
extern bool is_running_in_qemu;

// Loads infomation stored in system.cfg
// @return True if success, False if failed
bool load_kernal_configuration();

// Frees memory assoicated with system.cfg
// @note To be called during shutdown
void free_kernal_configuration();

#endif