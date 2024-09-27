#include "common/sys_calls.h"

int main() 
{
    sys_call_print("Hello, World!\n");

    sys_call_exit(0);
    return -1;
}