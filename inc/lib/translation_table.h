#ifndef TRANSLATION_TABLE_H
#define TRANSLATION_TABLE_H

#include "lib/page_allocator.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct translation_table_section_info
{
    page_allocation_info* allocation;
    uint16_t upper_attributes;
    uint16_t lowwer_attributes;
    void* section_start;
};

typedef struct translation_table_section_info translation_table_section_info;

struct translation_table_page_middle_directory_info
{
    uint64_t* page_middle_directory;
    uint8_t number_of_page_middle_directory_entrys;
};

typedef struct translation_table_page_middle_directory_info translation_table_page_middle_directory_info;

struct translation_table_info
{
    uint8_t number_of_page_upper_directory_entrys;

    uint64_t* page_global_directory;
    uint64_t* page_upper_directory;

    translation_table_page_middle_directory_info* page_middle_directorys;

    translation_table_section_info* sections;
    uint8_t number_of_sections;
};

typedef struct translation_table_info translation_table_info;

// initializes the translation_table_info struct and creates the correct buffers for the mmu
// @param table The table info struct to store the table in
// @param sections A pointer a array of translation_table_section_info to describe the managed space.
// This array is then copyed allowing for the input to be delete or be on the stack.
// Must be in assending order.
// @param number_of_sections The number of entrys in the sections array
// @return True, if success, False if failer
// @note This function does not update ttbr1_el1 or ttbr1_el0 that must be done elsewhere
bool initialize_translation_table(translation_table_info* table, translation_table_section_info* sections, uint8_t number_of_sections);

// Updates the buffers for the mmu for the given section
// @param table The table info struct
// @param section_id The index of the section
// @param only_update_active_buffers_when_ready Used to prevent the function from
// overwriting any buffers untill the new contents are ready, This is achived by allocating new buffers
// @return True, if success, False if failer
// @note This function does not update ttbr1_el1 or ttbr1_el0 that must be done elsewhere,
// however if this table is currently in use it will take imediate effect
// but to be safe the TLB still should be invalidated.
bool remake_translation_table_section(translation_table_info* table, int section_id, bool only_update_active_buffers_when_ready);

// Prints all translation table entrys of a given table to the UART
// @param table The table to print
void print_translation_table(translation_table_info* table);

// Updates the translation table to appened the new section
// @param table The table info struct
// @param section To add to the translation table
// @param only_update_active_buffers_when_ready Used to prevent the function from
// overwriting any buffers untill the new contents are ready, This is achived by allocating new buffers
// this paraeter is only ment to be used with the translation table controlls the code being activly ran
// @return True, if success, False if failer (many failed to find section) 
// @note This function does not update ttbr1_el1 or ttbr1_el0 that must be done elsewhere,
// however if this table is currently in use it will take imediate effect
// but to be safe the TLB still should be invalidated.
bool append_translation_table_section(translation_table_info* table, translation_table_section_info* section, bool only_update_active_buffers_when_ready);

// Calucates the ammount of memory mannaged by the translation table
// @param table The table to messure
// @return The number of bytes mannaged by the table
size_t get_translation_table_memory_mannaged(translation_table_info* table);

// Deletes the translation table
// @param table The table to delete
void destory_translation_table(translation_table_info* table);

#ifdef __cplusplus
}
#endif

#endif