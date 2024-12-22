.section .text

.extern _dac_output_start_internal

.global dac_output_start
dac_output_start:
    ldr     x3,     =dac_output_return_stack
    mov     x4,     sp
    str     x4,     [x3]
    ldr     x3,     =dac_output_return_address
    str     x30,     [x3]
    bl      _dac_output_start_internal


.global dac_output_end
dac_output_end:
    mov     x2,     xzr
    ldr     x1,     =dac_output_return_address
    ldr     x3,     [x1]     // Load return address
    str     x2,     [x1]     // Zero it
    cmp     x3,     x2
    beq     .dac_output_end_return      // Just return is its zero
    mov     x30,    x3
    ldr     x1,     =dac_output_return_stack
    ldr     x3,     [x1]    // Get stack page
    str     x2,     [x1]
    mov     sp,     x3
    br      x30
.dac_output_end_return:
    ret



.section .bss
.align 3 
 .global dac_output_return_stack
dac_output_return_stack:
.skip 8 

.align 3
.global dac_output_return_address
dac_output_return_address:
.skip 8