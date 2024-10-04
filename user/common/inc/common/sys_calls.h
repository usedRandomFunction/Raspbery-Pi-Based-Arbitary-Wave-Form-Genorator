#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H

// Prints the given string to the screen and uart
// @param str String to print
void sys_call_print(const char* str);

// Exits the current program with code i
// @param i Exit code
void sys_call_exit(int i);

// Prints the given charecter to the screen and uart
// @param c charecter to print
// @return returns EOF if failed
int sys_call_putchar(char c);

#endif