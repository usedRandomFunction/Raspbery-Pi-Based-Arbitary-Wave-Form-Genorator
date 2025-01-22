.section ".text.vectors"

.global _vectors

.extern system_call_undefined_handler
.extern kernal_arm_exception_handler
.extern user_arm_exception_handler
.extern system_call_exit

.macro vector_table_entry function
.align 7
b \function
.endm

.macro exception_handler_kernal_error id
.align  7
mov x0, \id
b       kernal_arm_exception_handler
.endm

.macro exception_handler_user_error id
.align  7
mov x0, \id
b        user_arm_exception_handler
.endm

.macro enter_from_syscall
sub	sp,     sp,     #16
stp x30,    x29,    [sp]
ldr x29,    =user_program_x30
str x30,    [x29]               // Used by some trace stuff to help with debuging
bl enter_from_syscall_internal
.endm

.macro enter_from_interupt
sub	sp,     sp,     #16
stp x30,    x29,    [sp]
bl enter_from_interupt_internal
.endm

.align	11
_vectors:
    // Exceptions for EL1t
    exception_handler_kernal_error #0   // synchronous
    vector_table_entry .irq             // IRQ
    exception_handler_kernal_error #2   // FIQ
    exception_handler_kernal_error #3   // SError

    // Exceptions for EL1h
    exception_handler_kernal_error #0   // synchronous
    vector_table_entry .irq             // IRQ
    exception_handler_kernal_error #2   // FIQ
    exception_handler_kernal_error #3   // SError

    // Exceptions for EL0 (AArch64)
    vector_table_entry .el0_svn         // synchronous
    vector_table_entry .irq             // IRQ
    exception_handler_user_error #2     // FIQ
    exception_handler_user_error #3     // SError

    // Exceptions for EL0 (AArch32)
    exception_handler_user_error #0     // synchronous
    vector_table_entry .irq             // IRQ
    exception_handler_user_error #2     // FIQ
    exception_handler_user_error #3     // SError

.irq:
    enter_from_interupt

    bl generic_irq_handler

    b exit_from_interupt


.el0_svn:
    enter_from_syscall
    mrs	x24,    esr_el1				// read the syndrome register
	lsr	x24,    x24,    #26		    // exception class
	cmp	x24,    #0x15			    // SVC in 64-bit state
	b.eq	    .el0_svc
    mov x0,     0
    bl       user_arm_exception_handler
    b exit_from_syscall
.el0_svc:
    lsl x8,    x8,     #3          // Sames as x8 = x8 * 8 (2^3)
    mrs x24,    esr_el1            
    and x24,    x24,    #0xFFFF     // Get SVC augrment
    lsl x24,    x24,    #3          // Sames as x24 = x24 * 8 (2^3)
                                    // Now we have the table offsets we can work
    cmp x24,    #0x40000            // Is it os or project syscalls
    bge .el0_svc_project_specfic
.el0_svc_os_functions:
    ldr x9,     =size_of_os_syscall_tables_sizes_table
    ldr x9,     [x9]
    cmp x24,     x9
    bge .el0_svc_failed             // Make sure we only look at the size of tables we have
    ldr x9,     =os_syscall_tables_sizes_table // Wow that name
    add x9,     x9,     x24
    ldr x9,     [x9]
    cmp x8,     x9                  
    bge .el0_svc_failed             // Now we know the syscall exists
    ldr x9,     =os_syscall_tables_table // Yup table of tables
    add x9,     x9,     x24
    ldr x9,     [x9]
    add x9,     x9,     x8                  
    ldr x9,     [x9]
    blr x9
    b exit_from_syscall
.el0_svc_project_specfic:
    sub x24,    x24,   #0x40000
    ldr x9,     =size_of_project_specfic_syscall_tables_sizes_table
    ldr x9,     [x9]
    cmp x24,     x9
    bge .el0_svc_failed             // Make sure we only look at the size of tables we have
    ldr x9,     =project_specfic_syscall_tables_sizes_table // Wow that name
    add x9,     x9,     x24
    ldr x9,     [x9]
    cmp x8,     x9                  
    bge .el0_svc_failed             // Now we know the syscall exists
    ldr x9,     =project_specfic_syscall_tables_table // Yup table of tables
    add x9,     x9,     x24
    ldr x9,     [x9]
    add x9,     x9,     x8                  
    ldr x9,     [x9]
    blr x9
    b exit_from_syscall
.el0_svc_failed:
    b       system_call_undefined_handler

enter_from_interupt_internal:
    sub	sp,     sp,     #16 * 15

    stp x0,     x1,     [sp, #16 * 0]
    stp x2,     x3,     [sp, #16 * 1]
    stp x4,     x5,     [sp, #16 * 2]
    stp x6,     x7,     [sp, #16 * 3]
    stp x8,     x9,     [sp, #16 * 4]
    stp x10,    x11,    [sp, #16 * 5]
    stp x12,    x13,    [sp, #16 * 6]
    stp x14,    x15,    [sp, #16 * 7]
    stp x16,    x17,    [sp, #16 * 8]
    stp x18,    x19,    [sp, #16 * 9]
    stp x20,    x21,    [sp, #16 * 10]
    stp x22,    x23,    [sp, #16 * 11]
    stp x24,    x25,    [sp, #16 * 12]
    stp x26,    x27,    [sp, #16 * 13]
    str x28,            [sp, #16 * 14]

    ret

exit_from_interupt:
    ldp x0,     x1,     [sp, #16 * 0]
    ldp x2,     x3,     [sp, #16 * 1]
    ldp x4,     x5,     [sp, #16 * 2]
    ldp x6,     x7,     [sp, #16 * 3]
    ldp x8,     x9,     [sp, #16 * 4]
    ldp x10,    x11,    [sp, #16 * 5]
    ldp x12,    x13,    [sp, #16 * 6]
    ldp x14,    x15,    [sp, #16 * 7]
    ldp x16,    x17,    [sp, #16 * 8]
    ldp x18,    x19,    [sp, #16 * 9]
    ldp x20,    x21,    [sp, #16 * 10]
    ldp x22,    x23,    [sp, #16 * 11]
    ldp x24,    x25,    [sp, #16 * 12]
    ldp x26,    x27,    [sp, #16 * 13]
    ldr x28,            [sp, #16 * 14]

    add	sp,     sp,     #16 * 15

    ldp x30,    x29,    [sp]
    add	sp, sp, #16

    eret


enter_from_syscall_internal:
    sub	sp,     sp,     #16 * 10

    stp x9,     x10,    [sp, #16 * 0]
    stp x11,    x12,    [sp, #16 * 1]
    stp x13,    x14,    [sp, #16 * 2]
    stp x15,    x16,    [sp, #16 * 3]
    stp x17,    x18,    [sp, #16 * 4]
    stp x19,    x20,    [sp, #16 * 5]
    stp x21,    x22,    [sp, #16 * 6]
    stp x23,    x24,    [sp, #16 * 7]
    stp x25,    x26,    [sp, #16 * 8]
    stp x27,    x28,    [sp, #16 * 9]

    ret

exit_from_syscall:
    ldp x9,     x10,    [sp, #16 * 0]
    ldp x11,    x12,    [sp, #16 * 1]
    ldp x13,    x14,    [sp, #16 * 2]
    ldp x15,    x16,    [sp, #16 * 3]
    ldp x17,    x18,    [sp, #16 * 4]
    ldp x19,    x20,    [sp, #16 * 5]
    ldp x21,    x22,    [sp, #16 * 6]
    ldp x23,    x24,    [sp, #16 * 7]
    ldp x25,    x26,    [sp, #16 * 8]
    ldp x27,    x28,    [sp, #16 * 9]

    add	sp, sp, #16 * 10

    ldp x30,    x29,    [sp]
    add	sp, sp, #16

    eret


.section .bss
.align 3 
.global user_program_x30
user_program_x30:
.skip 8
