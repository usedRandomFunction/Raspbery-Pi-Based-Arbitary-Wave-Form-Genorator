#ifndef USER_PROGRAM_H
#define USER_PROGRAM_H

#include "lib/translation_table.h"

struct user_program_info
{
    translation_table_info translation_table;
    uint32_t abi_version;
    void* stack_start;
    void* stack_end;
    void* entry;
};

typedef struct user_program_info user_program_info;

// Forces the current user program to exit
// and return int32_min
// @warning This function will force exuction to go 
// to where it would if the user called exit()
void terminate_current_user_program();

// Tells if there is a active user program
// @return True if there is a active user program
bool is_user_program_active();

// Loads a user program from the disk getting all infomation from its cfg file
// @param program A pointer to the user_program_info struct to fill out
// @param path Path to the cfg file
// @return True if success, false if failed
// @note While this program requires to change ttbr el0 to write to it
bool load_user_program_from_disk(user_program_info* program, const char* path);

// Creates the translation table for a program with two sections in memory
// 1. Program, including all code and data
// 2. Stack... is the stack
// @param program A pointer to the user_program_info struct to fill out
// @param program_start The address to load the program section into
// @param required_size The minium size of the program section
// @param entry Address to start execution
// @param required_stack the minimum size of the stack
// @return True if success, false if failed
bool initialize_monolithic_user_program(user_program_info* program, void* program_start, size_t required_size, void* entry, size_t required_stack);

// Changes whether or not the given program's program section is readonly
// @param program A pointer to the program to change
// @param readonly True to dissable writing, False to enable
// @return True if success, false if failed
bool set_monolithic_user_program_writability(user_program_info* program, bool readonly);

// Tells the mmu to use the program's translation table
// @param program The program to switch to
void switch_to_addess_space_to_program(user_program_info* program);

// Starts execution for the given user program
// @param program The program to execute
// @return return value of program
// @note After sys_call_exit() is called by the program this is the first function called
int execute_user_program(user_program_info* program);

// Removes the current user program and frees all resources associated
// @param program The program to free
void destroy_user_program(user_program_info* program);

// ===================================================================
// DO NOT CALL THIS FUNCTION DIRRECTLY USE execute_user_program INSTED
// ===================================================================
// Used internaly to call the user program and set the return address
// @param entry Address to switch to (In EL0)
// @param stack Address to set sp_el0 to
// @note DO NOT CALL THIS FUNCTION DIRRECTLY USE execute_user_program INSTED
void user_program_internal_use_program_excute(void* entry, void* stack);

#endif