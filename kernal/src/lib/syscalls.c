#include "lib/arm_exceptions.h"
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

void* const system_call_table[] = { system_call_print, system_call_exit };