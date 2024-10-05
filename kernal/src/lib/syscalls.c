#include "lib/arm_exceptions.h"
#include "io/putchar.h"
#include "io/printf.h"

// volatile void* SYSTEM_CALL_EXIT_RETURN_ADDRESS = NULL;
// int SYSTEM_CALL_EXIT_RETURN_VALUE = 0;

extern void system_call_exit(int i);

void system_call_print(const char* str)
{
    printf(str);
}

// void system_call_exit(int i)
// {
//     SYSTEM_CALL_EXIT_RETURN_VALUE = i;

//     asm volatile ("br %0"
// 	:
// 	: "r" (SYSTEM_CALL_EXIT_RETURN_ADDRESS));
// }

void system_call_putchar(char ch)
{
    putchar(ch);
}

void* const system_call_table[] = { system_call_print, system_call_exit, system_call_putchar };