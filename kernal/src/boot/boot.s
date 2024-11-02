.include "inc/boot/startUpDescriptorConstants.s"
.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #0xff
    // cpu id != 0, stop
    cbnz    x1, halt
    // cpu id == 0

prepare_for_el_switch:
    ldr     x1, =(1 << 20) // allow el1 to use SIMD registers
    msr     cpacr_el1, x1  // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500e/CIHBGEAB.html

    mrs     x1, currentel
    lsr     x1, x1, #2

    cmp     x1, #3
    beq     el3_entry // Jump to el3_entry if we are at el3

    cmp     x1, #2
    beq     el2_entry // Jump to el2_entry if we are at el2

    b       el1_entry // Else just jump to el1 and hope it works
el3_entry:

    // Initialize SCTLR_EL2 and HCR_EL2 to save values before entering EL2.
    msr     sctlr_el2, xzr
    msr     hcr_el2, xzr
    // Determine the EL2 Execution state.
    mrs     x0, scr_el3
    orr     x0, x0, #(1<<10) // RW EL2 Execution state is AArch64.
    orr     x0, x0, #(1<<0) // NS EL1 is Non-secure world.
    msr     scr_el3, x0
    mov     x0, #0b01001 // DAIF=0000
    msr     spsr_el3, x0 // M[4:0]=01001 EL2h must match SCR_EL3.RW
    // Determine EL2 entry.
    adr     x0, el2_entry // el2_entry points to the first instruction of
    msr     elr_el3, x0 // EL2 code.
    eret

el2_entry:
    // Set up exception handlers 
    ldr     x2, =_vectors
    msr     vbar_el1, x2

    // Initialize the SCTLR_EL1 register before entering EL1.
    msr     sctlr_el1, xzr
    // Determine the EL1 Execution state.
    
    mrs     x0, hcr_el2
    orr     x0, x0, #(1<<31) // RW=1 EL1 Execution state is AArch64.
    msr     hcr_el2, x0
    mov     x0, #0b00101 // DAIF=0000
    msr     spsr_el2, x0 // M[4:0]=00101 EL1h must match HCR_EL2.RW.
    adr     x0, el1_entry // el1_entry points to the first instruction of
    msr     elr_el2, x0 // EL1 code.
    eret

el1_entry:
    msr     daifclr, 0xf
    adrp	x0, __bss_start
    add     x0, x0, :lo12:__bss_start
	ldr	    x1, =__bss_size
	bl 	    memclr

init_mmu: // TODO when i have a better understanding of arm asm and the mmu remake this code myself
    // The mmu code is taken from https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson06/rpi-os.md
    .macro	create_pgd_entry, tbl, virt, tmp1, tmp2
    create_table_entry \tbl, \virt, PGD_SHIFT, \tmp1, \tmp2
    create_table_entry \tbl, \virt, PUD_SHIFT, \tmp1, \tmp2
    .endm

    .macro	create_table_entry, tbl, virt, shift, tmp1, tmp2
    lsr	\tmp1, \virt, #\shift
    and	\tmp1, \tmp1, #PTRS_PER_TABLE - 1			            // table index
    add	\tmp2, \tbl, #PAGE_SIZE
    orr	\tmp2, \tmp2, #MM_TYPE_PAGE_TABLE	
    str	\tmp2, [\tbl, \tmp1, lsl #3]
    add	\tbl, \tbl, #PAGE_SIZE					                // next level table page
    .endm

    .macro	create_block_map, tbl, phys, start, end, flags, tmp1
    lsr	\start, \start, #SECTION_SHIFT
    and	\start, \start, #PTRS_PER_TABLE - 1			            // table index
    lsr	\end, \end, #SECTION_SHIFT
    and	\end, \end, #PTRS_PER_TABLE - 1				            // table end index
    lsr	\phys, \phys, #SECTION_SHIFT
    mov	\tmp1, #\flags
    orr	\phys, \tmp1, \phys, lsl #SECTION_SHIFT			        // table entry
9999:	str	\phys, [\tbl, \start, lsl #3]				        // store the entry
    add	\start, \start, #1					                    // next entry
    add	\phys, \phys, #SECTION_SIZE				                // next block
    cmp	\start, \end
    b.ls	9999b
    .endm

    adrp	x0, boot_pg_dir
	mov	    x1, #PG_DIR_SIZE
	bl 	    memclr

    /* Get MMIO Base address and put it into x5 */
    bl get_board_type
    bl get_mmio_base_address
    mov x5, x0

	adrp	x0, boot_pg_dir
	mov	    x1, #VA_START 
	create_pgd_entry x0, x1, x2, x3

	/* Mapping kernel and init stack*/
	mov 	x1, xzr							                    // start mapping from physical offset 0
	mov 	x2, #VA_START						                // first virtual address
	ldr	    x3, =(VA_START - SECTION_SIZE)		                // last virtual address
    add     x3, x3, x5
	create_block_map x0, x1, x2, x3, MMU_FLAGS, x4

	/* Mapping device memory*/
    // TODO make this detect device base address	                    
    mov     x1, x5                                              // start mapping from device base address 
	mov 	x2, #VA_START						                // first virtual address
    add     x2, x2, x5
	ldr	    x3, =(VA_START + PHYS_MEMORY_SIZE - SECTION_SIZE)	// last virtual address
	create_block_map x0, x1, x2, x3, MMU_DEVICE_FLAGS, x4 

    // Set the top of the stack to the stack region
    ldr     x1, =_stack_top
    mov     sp, x1

    adrp	x0, boot_pg_dir	
	msr	    ttbr1_el1, x0
	msr	    ttbr0_el1, x0 // Basicly this needs to be added as i keept getting a prefetch abort just before jumping to the new addresses

	ldr	    x0, =(TCR_VALUE)		
	msr	    tcr_el1, x0 
    
	ldr	    x0, =(MAIR_VALUE)
	msr	    mair_el1, x0

    ldr     x2, =kernel_main
    ldr	    x0, =(SCTLR_MMU_ENABLED_WITH_CACHE)		
	msr	    sctlr_el1, x0

    
    // jump to C code, should not return
start_kernel:  
    blr      x2
    // for failsafe, halt this core too
    b       halt


halt:
    wfe
halt_loop:
    b       halt_loop
