.section .text

.global system_call_exit
system_call_exit:
    ldr x1,     =system_call_exit_return_value
    str w0,     [x1] // Store return value from program
    ldr x1,     =system_call_exit_return_address
    ldr x30,    [x1] // Load return address
    ldr x1,     =system_call_exit_return_stack
    ldr x0,     [x1] // Get stack page
    mov sp,     x0
    br x30



.section .bss
.align 3 
 .global system_call_exit_return_stack
system_call_exit_return_stack:
.skip 8 

.align 3
.global system_call_exit_return_address
system_call_exit_return_address:
.skip 8

.align 3 
.global system_call_exit_return_value
system_call_exit_return_value:
.skip 8
