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

typedef void (*USER_FUNCTION)(void);

enum
{
    VMEMMAP_WRITABILITY     =   1 << 0,     /* If set the allocation will be writeable*/
    VMEMMAP_NON_CACHABLE    =   1 << 1,     /* If set the allocation wont be cachable*/
    VMEMMAP_EXECUTABLE      =   1 << 2,     /* If set the allocation will be executable*/
    VMEMMAP_RETURN_POINTER  =   1 << 3,     /* If set vmemmap will use `ptr` as a void** given the kernal the abllity to chose the virtual address the address the mapping is given is returned use ptr. By defult the section will be placed in the first free block of memory, not including `0x000000000000`*/
};

// Used to indicate if switch_to was called
// should be free and set to NULL after the 
// app is executed
extern char* use_program_requested_switch;

// Forces the current user program to exit
// and return int32_min
// @warning This function will force exuction to go 
// to where it would if the user called exit()
void terminate_current_user_program();

// Resets file screen and input resources 
void on_user_program_exiting();

// Tells if there is a active user program
// @return True if there is a active user program
bool is_user_program_active();

// Gets the active user program
// @return pointer to active user program or null if there are none
user_program_info* get_active_user_program();

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
// @param stack_start The address of the start of the stack in memory
// @param required_stack the minimum size of the stack
// @return True if success, false if failed
bool initialize_monolithic_user_program(user_program_info* program, void* program_start, size_t required_size, void* entry, 
    void* stack_start, size_t required_stack);

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

// Used to manage virtual address translation within a user program
// @param program The program to change the mappings of
// @param ptr The address for the allocaion to goto
// @param size The size of the allocation (to create / modify), or set to zero to delete, a value of -1 will return the current size
// @param flags flags
// @return The size of the allocation in bytes / 0 if failed, or if deleting > 0 on success
size_t user_program_vmemmap(user_program_info* program, void* ptr, size_t size, int flags);

// Exits the current user program and starts the given program
// @param new_executable_path The path to the new program
// @note The next program will only be 
// @warning This function will force exuction to go 
// to where it would if the user called exit()
void user_program_switch_to(const char* new_executable_path);

// Runs the given function as the user and contiues execution after the user function has returned
// @param program The program to run as
// @param function The function to run
void execute_function_as_user_program(user_program_info* program, USER_FUNCTION function);

// =====================================
// DO NOT CALL THIS FUNCTION DIRRECTLY
// =====================================
// Called by user_arm_exception_handler, to check if
// a instruction abort was casued by _execute_function_as_user_program
// and return acordingly
// @return True, if the statment in the name is true, and _prepare_return_from_user_function was called, False if not
// @warning DO NOT CALL THIS FUNCTION DIRRECTLY
bool check_if_instruction_abort_is_user_program_function_return();

// ===================================================================
// DO NOT CALL THIS FUNCTION DIRRECTLY USE execute_user_program INSTED
// ===================================================================
// Used internaly to call the user program and set the return address
// @param entry Address to switch to (In EL0)
// @param stack Address to set sp_el0 to
// @warning DO NOT CALL THIS FUNCTION DIRRECTLY USE execute_user_program INSTED
void _execute_user_program(void* entry, void* stack);

// ================================================================================
// DO NOT CALL THIS FUNCTION DIRRECTLY USE _execute_function_as_user_program INSTED
// ================================================================================
// Used internaly to run the user function, and handle return
// @param function The function to run
// @warning DO NOT CALL THIS FUNCTION DIRRECTLY USE _execute_function_as_user_program INSTED
void _execute_function_as_user_program(USER_FUNCTION function);

// =====================================
// DO NOT CALL THIS FUNCTION DIRRECTLY
// =====================================
// ASM function for check_if_instruction_abort_is_user_program_function_return
// @warning DO NOT CALL THIS FUNCTION DIRRECTLY
void _return_from_user_function();
#endif
