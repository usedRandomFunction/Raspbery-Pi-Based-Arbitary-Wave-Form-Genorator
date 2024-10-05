.section .text

.global system_call_exit
system_call_exit:
    ldr x1,     =SYSTEM_CALL_EXIT_RETURN_VALUE
    str w0,     [x1] // Store return value from program
    ldr x1,     =SYSTEM_CALL_EXIT_RETURN_ADDRESS
    ldr x30,    [x1] // Load return address
    ldr x1,     =SYSTEM_CALL_EXIT_RETURN_STACK
    ldr x0,     [x1] // Get stack page
    mov sp,     x0
    br x30



.section .bss

    .align 3 
 .global SYSTEM_CALL_EXIT_RETURN_STACK
SYSTEM_CALL_EXIT_RETURN_STACK:
    .skip 8 

    .align 3
.global SYSTEM_CALL_EXIT_RETURN_ADDRESS
SYSTEM_CALL_EXIT_RETURN_ADDRESS:
    .skip 8

    .align 3 
.global SYSTEM_CALL_EXIT_RETURN_VALUE
SYSTEM_CALL_EXIT_RETURN_VALUE:
    .skip 8
