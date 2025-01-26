#ifndef BASIC_IO_H
#define BASIC_IO_H

// Prints the given string to the screen and uart
// @param str String to print
void printf(const char* str, ...);

// Puts the given charecter on the screen and uart
// @param c charecter to print
// @return returns EOF if failed
int putchar(char c);

#endif