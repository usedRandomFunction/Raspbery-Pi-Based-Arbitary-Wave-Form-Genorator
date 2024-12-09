.section ".text.reloader"

.extern MMIO_Base_Address
.extern uart_puts
.extern memcpy

.macro kernal_reloader_uart_get_u32
    mov     x0,     4
    bl      kernal_reloader_uart_get_word
.endm

.macro kernal_reloader_uart_get_u64
    mov     x0,     8
    bl      kernal_reloader_uart_get_word
.endm


// Used just after boot if prg_exit is depressed
.global begin_kernal_reload
begin_kernal_reload:
    ldr     x0,     =(0x20000000)               // Copy to 512 Mib in memory plenty out of the way
    adr     x1,     kernal_reloader_begin_relocatable
    mov     x2,     (kernal_reloader_end_relocatable - kernal_reloader_begin_relocatable)
    bl      memcpy
    adr     x0,     kernal_reload_begin_message
    bl      uart_puts
    ldr     x3,     =MMIO_Base_Address          // Get MMIO address so we can get the size
    ldr     x19,    [x3]
    ldr     x0,     =(0x200000 + 0x1000)
    add     x19,    x19,     x0                // Add offset since we care about the uart
    kernal_reloader_uart_get_u32
    mov     x20,    x0                         // Save into x20 for later
    adr     x0,     kernal_reload_ready_message
    bl      uart_puts
    ldr     x0,     =(0x20000000)              // Jump to the copy in memory
    br      x0

kernal_reload_begin_message:
.asciz "PRG_EXIT high, begining reload\nNext 4 bytes kernal size (big edin, multiple of 4).\n"
kernal_reload_ready_message:
.asciz "Reload ready!\n"

.align 2                            // Allign to 2^2 (4)

kernal_reloader_begin_relocatable:

// x20 is number of bytes to load x19 is uart address
kernal_reload_begin:
    mov     x0,         xzr
    msr     sctlr_el1,  x0      // At this point we dont need the MMU, it just gets in the way
    isb

    mov     x21,        0x80000    // Load address

.loop:
    kernal_reloader_uart_get_u32    // Get next 8 bytes
    rev     w0,         w0          // Need to change edinness
    str     w0,         [x21]       // Store it
    add     x21,        x21,    4
    subs    x20,        x20,    4   // Increment counters
    bne .loop                       // Keep going untill no more bytes need to be loaded

    mov     x0,     0x80000         // And jump to it
    br      x0

// Reads n bytes and as a big edin number
// argument x0 Is number of bytes to read
// x1, x2, x3 
// returns in x0
kernal_reloader_uart_get_word:
    mov     x0,     xzr
    mov     x3,     xzr
    mov     x1,     4               // Itteration counter
    mov     x2,     (1 << 4)        // Bit flag for reading data
.get_word_loop:
.get_word_wait_loop:
    ldr     w3,     [x19, #0x18]    // Load flag register
    ands    w3,     w3,     w2
    bne     .get_word_wait_loop     // Just wait for the data
    ldr     w3,     [x19, #0x00]    // Load data register
    lsl     x0,     x0,     #8      // Shift eight bits
    orr     x0,     x0,     x3      // Add this bit of data
    subs     x1,     x1,     #1
    bne     .get_word_loop          // Only lopo if x2 != 0
    ret

kernal_reloader_end_relocatable:
