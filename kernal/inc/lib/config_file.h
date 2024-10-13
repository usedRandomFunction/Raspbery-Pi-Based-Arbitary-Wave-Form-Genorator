#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "lib/dynamic_array.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct config_file_entry
{
    char* value_begin;
    char* value_end;

    bool back_slash_active;
};

typedef struct config_file_entry config_file_entry;

struct config_file
{
    char* buffer;

    dynamic_array entrys;
};

typedef struct config_file config_file;


// Loads the given config file from the disk and stores it in config*
// @param header Pointer to config_file struct to fill out
// @param path The path to the file
// @return True if success False if failed
// @note The file its self is closed after being loaded
bool load_config_file(config_file* header, const char* path);

// Frees all infomation surrounding the config file struct
// @param header Config file struct to free
void free_loaded_config_file(config_file* header);

// Returns a pointer to the entry with the given name
// @param header Config file to get the entry from
// @param name The name of the config to get
// @return Pominter to entry or null if failed
const config_file_entry* get_config_file_entry_from_name(config_file* header, const char* name);

// Returns the string stored in the given entry as a null-terminated string
// @param entry The entry to get the string from
// @return A pointer to the string if successfull or NULL if failed
char* get_string_from_config_file_entry_allocated(const config_file_entry* entry);

// Returns the string stored in the given entry as a null-terminated string 
// @param entry The entry to get the string from
// @param str The buffer to write the sting into
// @param dest_size size of str
// @return True if success, false if ran out of space
bool get_string_from_config_file_entry(const config_file_entry* entry, char* str, size_t dest_size);

// Returns the given entry as a uint64_t, supports binary, base 10, and hexadecimal
// @param entry The entry to get the number from
// @param error (Optional) used to show if a error occured (Ture if error)
// @return The number its self
uint64_t get_u64_from_config_file_entry(const config_file_entry* entry, bool* error);

// Returns the given entry as a uint64_t, supports binary, base 10, and hexadecimal
// @param entry The entry to get the number from
// @param error (Optional) used to show if a error occured (Ture if error)
// @return The number its self
int64_t get_s64_from_config_file_entry(const config_file_entry* entry, bool* error);

#endif