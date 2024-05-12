.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    // cpu id > 0, stop
    cbnz     x1, halt
    // cpu id == 0

prepare_for_el_switch:
    ldr x1, =(1 << 20) // allow el1 to use SIMD registers
    msr CPACR_EL1, x1  // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500e/CIHBGEAB.html

    mrs x1, CurrentEL
    lsr x1, x1, #2

    cmp x1, #3
    beq el3_entry // Jump to el3_entry if we are at el3

    cmp x1, #2
    beq el2_entry // Jump to el2_entry if we are at el2

    b el1_entry // Else just jump to el1 and hope it works
el3_entry:

    // Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2.
    msr SCTLR_EL2, XZR
    msr HCR_EL2, XZR
    // Determine the EL2 Execution state.
    mrs X0, SCR_EL3
    orr X0, X0, #(1<<10) // RW EL2 Execution state is AArch64.
    orr X0, X0, #(1<<0) // NS EL1 is Non-secure world.
    msr SCR_EL3, x0
    mov X0, #0b01001 // DAIF=0000
    msr SPSR_EL3, X0 // M[4:0]=01001 EL2h must match SCR_EL3.RW
    // Determine EL2 entry.
    adr X0, el2_entry // el2_entry points to the first instruction of
    msr ELR_EL3, X0 // EL2 code.
    eret

el2_entry:
    // Initialize the SCTLR_EL1 register before entering EL1.
    msr SCTLR_EL1, XZR
    // Determine the EL1 Execution state.
    
    mrs X0, HCR_EL2
    orr X0, X0, #(1<<31) // RW=1 EL1 Execution state is AArch64.
    msr HCR_EL2, X0
    mov X0, #0b00101 // DAIF=0000
    msr SPSR_EL2, X0 // M[4:0]=00101 EL1h must match HCR_EL2.RW.
    adr X0, el1_entry // el1_entry points to the first instruction of
    msr ELR_EL2, X0 // EL1 code.
    eret

el1_entry:
    // clear bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_size
bss_clear:  cbz     x2, start_kernel
    str     xzr, [x1], #8
    sub     x2, x2, #1
    cbnz    x2, bss_clear

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

    // jump to C code, should not return
start_kernel:  
    bl      kernel_main
    // for failsafe, halt this core too
    b       halt

halt:
    wfe
halt_loop:
    b halt_loop
