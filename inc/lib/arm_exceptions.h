#ifndef ARM_EXCEPTIONS_H
#define ARM_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

// Prints debug message and halts the program
void kernel_panic();

// By the code to handle arm exceptions,
// This function is not suposted to be called by c code, only the vector table
// @param type The type of exception
// @note Calling this function halts execution
void arm_exception_handler(unsigned long type);

#ifdef __cplusplus
}
#endif

#endif