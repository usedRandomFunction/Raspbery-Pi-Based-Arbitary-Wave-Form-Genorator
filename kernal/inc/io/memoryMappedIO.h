#ifndef MEMORY_MAPPED_IO_H
#define MEMORY_MAPPED_IO_H

#include <stddef.h>
#include <stdint.h>



extern volatile size_t MMIO_Base_Address;


// Sets the mmio base address to the address that the given board type has
// @param boardType The boardType number of the current system
void set_mmio_base(int boardType);

// Gets the mmio base address to the address that the given board type has
// @param boardType The boardType number of the current system
// @return the base address as number not a pointer
size_t get_mmio_base_address(int boardType);

// Reads the value of the given memory mapped register
// @param reg The address of the register as a offset from the base address
// @return The value of the register
inline uint32_t mmio_read(size_t reg);

// Writes to a given memory mapped register
// @param reg The address of the register as a offset from the base address
// @param data The value to be written to the register
inline void mmio_write(size_t reg, uint32_t data);

// Performs a bitwise or on a given memory mapped register
// @param reg The address of the register as a offset from the base address
// @param rhs The right hand size of the bitwise operator
inline void mmio_write_bitwise_or(size_t reg, uint32_t rhs);

// Performs a bitwise and on a given memory mapped register
// @param reg The address of the register as a offset from the base address
// @param rhs The right hand size of the bitwise operator
inline void mmio_write_bitwise_and(size_t reg, uint32_t rhs);

// Performs a bitwise xor on a given memory mapped register
// @param reg The address of the register as a offset from the base address
// @param rhs The right hand size of the bitwise operator

inline void mmio_write_bitwise_xor(size_t reg, uint32_t rhs);

// Performs a bitwise not on a given memory mapped register
// @param reg The address of the register as a offset from the base address
inline void mmio_write_bitwise_not(size_t reg);

// Writes n bits to a memory mapped register at a offset
// @param reg The address of the register as a offset from the base address
// @param data The value to be written to the register
// @param offset The lelft bit shift to be preformed on the data
// @param size The number of bits to write
inline void mmio_write_offset_of_size(size_t reg, uint32_t data, uint_fast8_t offset, uint_fast8_t size);

// Calcuates the pointer to a memory mapped register
// @param reg The address of the register as a offset from the base address
// @return A pointer to the register
inline volatile uint32_t* get_mmio_pointer(size_t reg);

inline uint32_t mmio_read(size_t reg)
{
	return *(volatile uint32_t*)(MMIO_Base_Address + reg);
}

inline void mmio_write(size_t reg, uint32_t data)
{
	*(volatile uint32_t*)(MMIO_Base_Address + reg) = data;
}
 
inline void mmio_write_bitwise_or(size_t reg, uint32_t rhs)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) |= rhs;
}

inline void mmio_write_bitwise_and(size_t reg, uint32_t rhs)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) &= rhs;
}

inline void mmio_write_bitwise_xor(size_t reg, uint32_t rhs)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) ^= rhs;
}

inline void mmio_write_bitwise_not(size_t reg)
{
    *(volatile uint32_t*)(MMIO_Base_Address + reg) = ~*(volatile uint32_t*)(MMIO_Base_Address + reg);
}

inline void mmio_write_offset_of_size(size_t reg, uint32_t data, uint_fast8_t offset, uint_fast8_t size)
{
    uint32_t bitMask = (1 << size) - 1;
    bitMask <<= offset;
    data <<= offset;

    uint32_t value = mmio_read(reg);
    value &= ~bitMask;
    value |= data;

    mmio_write(reg, value);
}

inline volatile uint32_t* get_mmio_pointer(size_t reg)
{
    return (volatile uint32_t*)(MMIO_Base_Address + reg);
}


enum
{
    // The offsets for reach register.
    GPIO_BASE           = 0x200000,
    
    // GPIO Function Select
    GPFSEL0             = (GPIO_BASE + 0x00),
    GPFSEL1             = (GPIO_BASE + 0x04),
    GPFSEL2             = (GPIO_BASE + 0x08),
    GPFSEL3             = (GPIO_BASE + 0x0C),
    GPFSEL4             = (GPIO_BASE + 0x10),
    GPFSEL5             = (GPIO_BASE + 0x14),

    // GPIO Output set
    GPSET0              = (GPIO_BASE + 0x1C),
    GPSET1              = (GPIO_BASE + 0x20),

    // GPIO Output clear
    GPCLR0              = (GPIO_BASE + 0x28),
    GPCLR1              = (GPIO_BASE + 0x2C),

    // GPIO Pin level
    GPLEV0              = (GPIO_BASE + 0x34),
    GPLEV1              = (GPIO_BASE + 0x38),

    // GPIO Pin Event Detect Status
    GPEDS0              = (GPIO_BASE + 0x40),
    GPEDS1              = (GPIO_BASE + 0x44),

    // GPIO Pin Rising Edge Detect Enable
    GPREN0              = (GPIO_BASE + 0x4C),
    GPREN1              = (GPIO_BASE + 0x50),

    // GPIO Pin Falling Edge Detect Enable
    GPFEN0              = (GPIO_BASE + 0x58),
    GPFEN1              = (GPIO_BASE + 0x5C),

    // GPIO Pin High Detect Enable
    GPHEN0              = (GPIO_BASE + 0x64),
    GPHEN1              = (GPIO_BASE + 0x68),

    // GPIO Pin Low Detect Enable
    GPLEN0              = (GPIO_BASE + 0x70),
    GPLEN1              = (GPIO_BASE + 0x74),

    // GPIO Pin Async. Rising Edge Detect
    GPAREN0             = (GPIO_BASE + 0x7C),
    GPAREN1             = (GPIO_BASE + 0x80),

    // GPIO Pin Async. Falling Edge Detect
    GPAFEN0             = (GPIO_BASE + 0x88),
    GPAFEN1             = (GPIO_BASE + 0x8C),

    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD               = (GPIO_BASE + 0x94),
 
    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0           = (GPIO_BASE + 0x98),
    GPPUDCLK1           = (GPIO_BASE + 0x9C),
 
    // The base address for UART.
    UART0_BASE          = (GPIO_BASE + 0x1000),
    
    // The offsets for reach register for the UART.
    UART0_DR            = (UART0_BASE + 0x00),
    UART0_RSRECR        = (UART0_BASE + 0x04),
    UART0_FR            = (UART0_BASE + 0x18),
    UART0_ILPR          = (UART0_BASE + 0x20),
    UART0_IBRD          = (UART0_BASE + 0x24),
    UART0_FBRD          = (UART0_BASE + 0x28),
    UART0_LCRH          = (UART0_BASE + 0x2C),
    UART0_CR            = (UART0_BASE + 0x30),
    UART0_IFLS          = (UART0_BASE + 0x34),
    UART0_IMSC          = (UART0_BASE + 0x38),
    UART0_RIS           = (UART0_BASE + 0x3C),
    UART0_MIS           = (UART0_BASE + 0x40),
    UART0_ICR           = (UART0_BASE + 0x44),
    UART0_DMACR         = (UART0_BASE + 0x48),
    UART0_ITCR          = (UART0_BASE + 0x80),
    UART0_ITIP          = (UART0_BASE + 0x84),
    UART0_ITOP          = (UART0_BASE + 0x88),
    UART0_TDR           = (UART0_BASE + 0x8C),
    
    // SPI0
    SPI0_BASE           = (GPIO_BASE + 0x4000),

    SPI0_CS             = (SPI0_BASE + 0x00),
    SPI0_FIFO           = (SPI0_BASE + 0x04),
    SPI0_CLK            = (SPI0_BASE + 0x08),
    SPI0_DLEN           = (SPI0_BASE + 0x0C),
    SPI0_LTOH           = (SPI0_BASE + 0x10),
    SPI0_DC             = (SPI0_BASE + 0x14),

    // Mailbox Registers
    MBOX_BASE           = 0xB880,
    MBOX_READ           = (MBOX_BASE + 0x00),
    MBOX_STATUS         = (MBOX_BASE + 0x18),
    MBOX_WRITE          = (MBOX_BASE + 0x20),

    // Hardwere random
    RNG_BASE            = 0x104000, 
    RNG_CTRL            = (RNG_BASE + 0x00),
    RNG_STATUS          = (RNG_BASE + 0x04),
    RNG_DATA            = (RNG_BASE + 0x08),
    RNG_INT_MASK        = (RNG_BASE + 0x10),

    // Auxiliary Peripheral Registers
    AUX_Base            = 0x215000,
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

    // Embedded MultiMediaCard  Registers'
    EMMC_BASE           = 0x00300000,
    EMMC_ARG2           = (EMMC_BASE + 0x00),
    EMMC_BLKSIZECNT     = (EMMC_BASE + 0x04),
    EMMC_ARG1           = (EMMC_BASE + 0x08),
    EMMC_CMDTM          = (EMMC_BASE + 0x0C),
    EMMC_RESP0          = (EMMC_BASE + 0x10),
    EMMC_RESP1          = (EMMC_BASE + 0x14),
    EMMC_RESP2          = (EMMC_BASE + 0x18),
    EMMC_RESP3          = (EMMC_BASE + 0x1C),
    EMMC_DATA           = (EMMC_BASE + 0x20),
    EMMC_STATUS         = (EMMC_BASE + 0x24),
    EMMC_CONTROL0       = (EMMC_BASE + 0x28),
    EMMC_CONTROL1       = (EMMC_BASE + 0x2C),
    EMMC_INTERRUPT      = (EMMC_BASE + 0x30),
    EMMC_INT_MASK       = (EMMC_BASE + 0x34),
    EMMC_INT_EN         = (EMMC_BASE + 0x38),
    EMMC_CONTROL2       = (EMMC_BASE + 0x3C),
    EMMC_SLOTISR_VER    = (EMMC_BASE + 0xFC),

    // System timer registers
    SYS_TIMER_BASE  = 0x3000,
    SYS_TIMER_CS    = (SYS_TIMER_BASE + 0x00),
    SYS_TIMER_CLO   = (SYS_TIMER_BASE + 0x04),
    SYS_TIMER_CHI   = (SYS_TIMER_BASE + 0x08),
    SYS_TIMER_C0    = (SYS_TIMER_BASE + 0x0C),
    SYS_TIMER_C1    = (SYS_TIMER_BASE + 0x10),
    SYS_TIMER_C2    = (SYS_TIMER_BASE + 0x14),
    SYS_TIMER_C3    = (SYS_TIMER_BASE + 0x18),

    // Interrupt controlls
    IRQ_REGISTERS_A_BASE    = 0xB200,
    IRQ_REGISTERS_B_BASE    = 0x1000000,
    IRQ_BASIC_PENDING       = (IRQ_REGISTERS_A_BASE + 0x00),
    IRQ_PENDING_1           = (IRQ_REGISTERS_A_BASE + 0x04),
    IRQ_PENDING_2           = (IRQ_REGISTERS_A_BASE + 0x08),
    FIQ_CTRL                = (IRQ_REGISTERS_A_BASE + 0x0C),
    IRQ_ENABLE_1            = (IRQ_REGISTERS_A_BASE + 0x10),
    IRQ_ENABLE_2            = (IRQ_REGISTERS_A_BASE + 0x14),
    IRQ_ENABLE_BASE         = (IRQ_REGISTERS_A_BASE + 0x18),
    IRQ_DISABLE_1           = (IRQ_REGISTERS_A_BASE + 0x1C),
    IRQ_DISABLE_2           = (IRQ_REGISTERS_A_BASE + 0x20),
    IRQ_DISABLE_BASE        = (IRQ_REGISTERS_A_BASE + 0x24),
    GPU_IRQ_ROUTING         = (IRQ_REGISTERS_B_BASE + 0x0C),
    CORE_0_TIMER_IRQ_CTRL   = (IRQ_REGISTERS_B_BASE + 0x40),
    CORE_1_TIMER_IRQ_CTRL   = (IRQ_REGISTERS_B_BASE + 0x44),
    CORE_2_TIMER_IRQ_CTRL   = (IRQ_REGISTERS_B_BASE + 0x48),
    CORE_3_TIMER_IRQ_CTRL   = (IRQ_REGISTERS_B_BASE + 0x4C),
    CORE_0_MAILBOX_IRQ_CTRL = (IRQ_REGISTERS_B_BASE + 0x50),
    CORE_1_MAILBOX_IRQ_CTRL = (IRQ_REGISTERS_B_BASE + 0x54),
    CORE_2_MAILBOX_IRQ_CTRL = (IRQ_REGISTERS_B_BASE + 0x58),
    CORE_3_MAILBOX_IRQ_CTRL = (IRQ_REGISTERS_B_BASE + 0x5C),
    CORE_0_IRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x60),
    CORE_1_IRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x64),
    CORE_2_IRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x68),
    CORE_3_IRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x6C),
    CORE_0_FRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x60),
    CORE_1_FRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x64),
    CORE_2_FRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x68),
    CORE_3_FRQ_SOURCE       = (IRQ_REGISTERS_B_BASE + 0x6C),
};



#endif