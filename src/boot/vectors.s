.section ".text"

.global _vectors

.align	11
_vectors:
    // Current EL with SP0
    // synchronous
    .align  7
    mov     x0, #0
    b       arm_exception_handler

    // IRQ
    .align  7
    mov     x0, #1
    b       arm_exception_handler

    // FIQ
    .align  7
    mov     x0, #2
    b       arm_exception_handler

    // SError
    .align  7
    mov     x0, #3
    b       arm_exception_handler

    // Current EL with SPx
    // synchronous
    .align  7
    mov     x0, #0
    b       arm_exception_handler

    // IRQ
    .align  7
    mov     x0, #1
    b       arm_exception_handler

    // FIQ
    .align  7
    mov     x0, #2
    b       arm_exception_handler

    // SError
    .align  7
    mov     x0, #3
    b       arm_exception_handler

    // Lower EL using AArch64
    // synchronous
    .align  7
    mov     x0, #0
    b       arm_exception_handler

    // IRQ
    .align  7
    mov     x0, #1
    b       arm_exception_handler

    // FIQ
    .align  7
    mov     x0, #2
    b       arm_exception_handler

    // SError
    .align  7
    mov     x0, #3
    b       arm_exception_handler

    // Lower EL using AArch32
    // synchronous
    .align  7
    mov     x0, #0
    b       arm_exception_handler

    // IRQ
    .align  7
    mov     x0, #1
    b       arm_exception_handler

    // FIQ
    .align  7
    mov     x0, #2
    b       arm_exception_handler

    // SError
    .align  7
    mov     x0, #3
    b       arm_exception_handler
