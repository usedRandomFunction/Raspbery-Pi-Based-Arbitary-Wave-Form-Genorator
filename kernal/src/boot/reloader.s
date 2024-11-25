.section ".text.reloader"

.extern MMIO_Base_Address
.extern uart_puts
.extern memcpy

// Used just after boot if prg_exit is depressed
// should still be in el2 or 3
.global begin_kernal_reload
begin_kernal_reload:
    ldr     x0,     =(0x20000000)               // Copy to 512 Mib in memory plenty out of the way
    adr     x1,     kernal_reloader_begin_relocatable
    mov     x2,     (kernal_reloader_end_relocatable - kernal_reloader_begin_relocatable)
    bl      memcpy
    adr     x0,     kernal_reload_begin_message
    bl      uart_puts
    ldr     x3,     =MMIO_Base_Address          // Get MMIO address so we can get the size
    ldr     x1,     [x3]
    ldr     x0,     =(0x200000 + 0x1000)
    add     x1,     x1,     x0                  // Add offset since we care about the uart
    bl      kernal_reloader_uart_get_u32
    mov     x5,     x0                          // Save into x5 for lat
    adr     x0,     kernal_reload_ready_message
    bl      uart_puts
    ldr     x0,     =(0x20000000)              // Jump to the copy in memory
    br      x0

// Reads 4 bytes and as a big edin number
// argument x1 is uart base address, x2, x3, x4 are working register
// returns in x0
kernal_reloader_uart_get_u32:
    mov     x0,     xzr
    mov     x4,     xzr
    mov     x2,     4               // Itteration counter
    mov     x3,     (1 << 4)        // Bit flag for reading data
.get_u32_loop:
.get_u32_wait_loop:
    ldr     w4,     [x1, #0x18]     // Load flag register
    ands    w4,     w4,     w3
    bne     .get_u32_wait_loop      // Just wait for the data
    ldr     w4,     [x1, #0x00]     // Load data register
    lsl     x0,     x0,     #8      // Shift eight bits
    orr     x0,     x0,     x4      // Add this bit of data
    subs     x2,     x2,     #1
    bne     .get_u32_loop           // Only lopo if x2 != 0
    ret

kernal_reload_begin_message:
.asciz "PRG_EXIT high, begining reload\nNext 4 bytes kernal size (big edin).\n"
kernal_reload_ready_message:
.asciz "Reload ready!\n"

.align 2                            // Allign to 2^2 (4)

kernal_reloader_begin_relocatable:

// x5 is number of bytes to load x1 is uart address
kernal_reload_begin:
    mov     x6,     xzr             // load address
    mov     x2,     (1 << 4)        // Bit flag for reading data
.reload_loop:
.wait_for_data:
    ldr     x4,     [x1, #0x18]     // Load flag register
    ands    x4,     x4,     x3
    bne     .wait_for_data          // Just wait for the data
    ldr     x0,     [x1, #0x00]     // Read data from uart
    strb    w0,     [x6]
    add     x6,     x6,     #1
    subs    x5,     x5,     #1
    bne     .reload_loop            // Keep looping untill all data copyed



    mov     x0,     xzr             // And jump to it
    br      x0

kernal_reloader_end_relocatable:
