#include "lib/interrupts.h"

#include "io/memoryMappedIO.h"
#include "lib/exceptions.h"
#include "io/printf.h"

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
    int irq = 0;

    if (irq_source & (1 << 1))     // Handle physical timer
    {
        irq = 30;
    }
    else                            // Handle GPU interupt
    {
        uint64_t irq_raw = mmio_read(IRQ_PENDING_1) | ((uint64_t)mmio_read(IRQ_PENDING_2) << 32);

        while (irq_raw) // Get first pending interupt and get its id
        {
            if (irq_raw & 0b1)
                break;

            irq++;

            irq_raw >>= 1;
        }
    }

    
    // TODO handler

    printf("\n!============================!\nUnkown IRQ %d\n", irq);
    kernel_panic();
}