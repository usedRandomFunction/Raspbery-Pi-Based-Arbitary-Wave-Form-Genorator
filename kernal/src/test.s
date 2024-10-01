.section .text
.global test_el0
test_el0:
    mov x1, #0x00
    msr spsr_el1, x1

    msr elr_el1, x0

    eret
    ret