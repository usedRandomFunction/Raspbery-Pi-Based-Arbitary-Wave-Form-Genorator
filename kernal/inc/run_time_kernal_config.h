#ifndef RUN_TIME_KERNAL_CONFIG_H
#define RUN_TIME_KERNAL_CONFIG_H

#include <stdbool.h>

extern char* main_interface_app_path;

// Loads infomation stored in system.cfg
// @return True if success, False if failed
bool load_kernal_configuration();

// Frees memory assoicated with system.cfg
// @note To be called during shutdown
void free_kernal_configuration();

#endif