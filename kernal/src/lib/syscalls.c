#include "lib/arm_exceptions.h"
#include "io/putchar.h"
#include "io/printf.h"

void system_call_print(const char* str)
{
    printf(str);
}

void system_call_exit(int i)
{
    // TODO
    printf("SYSCALL EXIT!!!");
    kernel_panic();
}

void system_call_putchar(char ch)
{
    putchar(ch);
}

void* const system_call_table[] = { system_call_print, system_call_exit, system_call_putchar };