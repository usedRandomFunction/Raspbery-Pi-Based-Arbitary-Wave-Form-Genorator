#ifndef MEMORY_MAPPED_IO_H
#define MEMORY_MAPPED_IO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern size_t MMIO_Base_Address;



// Memory-Mapped I/O input
inline uint32_t mmio_read(size_t reg)
{
	return *(volatile uint32_t*)(MMIO_Base_Address + reg);
}
// Memory-Mapped I/O output
inline void mmio_write(size_t reg, uint32_t data)
{
	*(volatile uint32_t*)(MMIO_Base_Address + reg) = data;
}
 
// Memory-Mapped I/O output
inline void mmio_write_bitwise_or(size_t reg, uint32_t data)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) |= data;
}

// Memory-Mapped I/O output
inline void mmio_write_bitwise_and(size_t reg, uint32_t data)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) &= data;
}

// Memory-Mapped I/O output
inline void mmio_write_bitwise_xor(size_t reg, uint32_t data)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) ^= data;
}

// Memory-Mapped I/O output
inline void mmio_write_bitwise_not(size_t reg)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) = ~*(volatile uint32_t*)(MMIO_Base_Address + reg);
}

inline void mmio_write_offset_of_size(size_t reg, uint32_t data, uint_fast8_t offset, uint_fast8_t size)
{
    uint32_t bitMask = (1 << size) - 1;
    bitMask <<= offset;
    data <<= offset;

    mmio_write_bitwise_and(reg, ~bitMask);
    mmio_write_bitwise_or(reg, data);
}

inline volatile uint32_t* get_mmio_pointer(size_t reg)
{
    return (volatile uint32_t*)(MMIO_Base_Address + reg);
}

void set_mmio_base(int boardType);

enum
{
    // The offsets for reach register.
    GPIO_BASE = 0x200000,
    
    // GPIO Function Select
    GPFSEL0 = (GPIO_BASE + 0x00),
    GPFSEL1 = (GPIO_BASE + 0x04),
    GPFSEL2 = (GPIO_BASE + 0x08),
    GPFSEL3 = (GPIO_BASE + 0x0C),
    GPFSEL4 = (GPIO_BASE + 0x10),
    GPFSEL5 = (GPIO_BASE + 0x14),

    // GPIO Output set
    GPSET0 = (GPIO_BASE + 0x1C),
    GPSET1 = (GPIO_BASE + 0x20),

    // GPIO Output clear
    GPCLR0 = (GPIO_BASE + 0x28),
    GPCLR1 = (GPIO_BASE + 0x2C),

    // GPIO Pin level
    GPLEV0 = (GPIO_BASE + 0x34),
    GPLEV1 = (GPIO_BASE + 0x38),

    // GPIO Pin Event Detect Status
    GPEDS0 = (GPIO_BASE + 0x40),
    GPEDS1 = (GPIO_BASE + 0x44),

    // GPIO Pin Rising Edge Detect Enable
    GPREN0 = (GPIO_BASE + 0x4C),
    GPREN1 = (GPIO_BASE + 0x50),

    // GPIO Pin Falling Edge Detect Enable
    GPFEN0 = (GPIO_BASE + 0x58),
    GPFEN1 = (GPIO_BASE + 0x5C),

    // GPIO Pin High Detect Enable
    GPHEN0 = (GPIO_BASE + 0x64),
    GPHEN1 = (GPIO_BASE + 0x68),

    // GPIO Pin Low Detect Enable
    GPLEN0 = (GPIO_BASE + 0x70),
    GPLEN1 = (GPIO_BASE + 0x74),


    // GPIO Pin Async. Rising Edge Detect
    GPAREN0 = (GPIO_BASE + 0x7C),
    GPAREN1 = (GPIO_BASE + 0x80),

    // GPIO Pin Async. Falling Edge Detect
    GPAFEN0 = (GPIO_BASE + 0x88),
    GPAFEN1 = (GPIO_BASE + 0x8C),

    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),
 
    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),
 
    // The base address for UART.
    UART0_BASE = (GPIO_BASE + 0x1000), // for raspi4 0xFE201000, raspi2 & 3 0x3F201000, and 0x20201000 for raspi1
    
    // The offsets for reach register for the UART.
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
 
    // The offsets for Mailbox registers
    MBOX_BASE    = 0xB880,
    MBOX_READ    = (MBOX_BASE + 0x00),
    MBOX_STATUS  = (MBOX_BASE + 0x18),
    MBOX_WRITE   = (MBOX_BASE + 0x20),

    // Auxiliary Peripheral Registers
    AUX_Base = 0x215000,
    AUX_IRQ             = (AUX_Base + 0x00),
    AUX_ENABLES         = (AUX_Base + 0x04),
    AUX_MU_IO_REG       = (AUX_Base + 0x40),
    AUX_MU_IER_REG      = (AUX_Base + 0x44),
    AUX_MU_IIR_REG      = (AUX_Base + 0x48),
    AUX_MU_LCR_REG      = (AUX_Base + 0x4C),
    AUX_MU_MCR_REG      = (AUX_Base + 0x50),
    AUX_MU_LSR_REG      = (AUX_Base + 0x54),
    AUX_MU_MSR_REG      = (AUX_Base + 0x58),
    AUX_MU_SCRATCH      = (AUX_Base + 0x5C),
    AUX_MU_CNTL_REG     = (AUX_Base + 0x60),
    AUX_MU_STAT_REG     = (AUX_Base + 0x64),
    AUX_MU_BAUD_REG     = (AUX_Base + 0x68),
    AUX_SPI0_CNTL0_REG  = (AUX_Base + 0x80),
    AUX_SPI0_CNTL1_REG  = (AUX_Base + 0x84),
    AUX_SPI0_STAT_REG   = (AUX_Base + 0x88),
    AUX_SPI0_IO_REG     = (AUX_Base + 0x90),
    AUX_SPI0_PEEK_RE    = (AUX_Base + 0x94),
    AUX_SPI1_CNTL0_REG  = (AUX_Base + 0xC0),
    AUX_SPI1_CNTL1_RE   = (AUX_Base + 0xC4),
    AUX_SPI1_STAT_REG   = (AUX_Base + 0xC8),
    AUX_SPI1_IO_REG     = (AUX_Base + 0xD0),
    AUX_SPI1_PEEK_REG   = (AUX_Base + 0xD4),

};

#ifdef __cplusplus
}
#endif

#endif