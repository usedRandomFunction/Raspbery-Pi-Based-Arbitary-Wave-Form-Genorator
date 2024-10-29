#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H



// Prints debug message and halts the program
// @note Calling this function halts execution
void kernel_panic();

// Used when a kernal function wants to kill a user program 
// due to a error during a syscall, prints infomation about 
// the error as well
// @param fmt format like printf
// @param ... agian like printf
void generic_user_exception(const char* fmt, ...);

#endif