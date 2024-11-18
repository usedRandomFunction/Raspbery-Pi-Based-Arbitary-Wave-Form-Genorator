#include "lib/interrupts.h"

#include "io/memoryMappedIO.h"
#include "lib/exceptions.h"
#include "io/keypad.h"
#include "io/printf.h"
#include "io/gpio.h"

void enable_irq(int id)
{
    if (id <= 31)
        mmio_write(IRQ_ENABLE_1, 1 << id);
    else
        mmio_write(IRQ_ENABLE_2, 1 << (id - 32));
}

void disable_irq(int id)
{
    if (id <= 31)
        mmio_write(IRQ_DISABLE_1, 1 << id);
    else
        mmio_write(IRQ_DISABLE_2, 1 << (id - 32));
}

void route_gpu_irqs(int core)
{
    mmio_write_offset_of_size(GPU_IRQ_ROUTING,  // Write to the routing
        core & 0b11,                            // Only take first two bits
        0,                                      // Offset zero
        2);                                     // 2 Bits
}

// Used by vectors.s so not static even though its not in the header
void generic_irq_handler()
{   
    uint32_t irq_source = mmio_read(CORE_0_IRQ_SOURCE); // TODO check if is core other then zero
    int irq = -1;

    if (irq_source & (1 << 1))     // Handle physical timer
    {
        irq = 30;
    }
    else                            // Handle GPU interupt
    {
        uint64_t irq_raw = mmio_read(IRQ_PENDING_1) | (((uint64_t)mmio_read(IRQ_PENDING_2)) << 32);

        if (irq_raw)
            irq = __builtin_ctzll(irq_raw);
    }

    if (irq == 57)
        keypad_uart_interupt_handler();
    else if (irq == 52)
        gpio_interupt_handler_function();
    else
    {
        // TODO handler

        printf("\n!============================!\nUnkown IRQ %d\n", irq);
        kernel_panic();
    }
}