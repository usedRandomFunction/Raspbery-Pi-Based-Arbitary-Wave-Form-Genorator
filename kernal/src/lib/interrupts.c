#include "lib/interrupts.h"

#include "io/memoryMappedIO.h"
#include "lib/user_program.h"
#include "lib/exceptions.h"
#include "lib/memory.h"
#include "lib/events.h"
#include "io/keypad.h"
#include "io/printf.h"
#include "io/gpio.h"

bool interupt_active;

static USER_INTERUPT_HANDLER s_user_interupt_handlers[MAX_NUMBER_OF_USER_INTERUPT_HANDLERS];

void initialize_interupts()
{
    mmio_write(IRQ_DISABLE_1, 0xFFFFFFFF);      // Dissable all irqs
    mmio_write(IRQ_DISABLE_2, 0xFFFFFFFF);
    route_gpu_irqs(0);                          // Sets the core to handle gpu irqs
    
    mmio_write(CORE_0_TIMER_IRQ_CTRL, 0);       // Turn off the timer

    memset(s_user_interupt_handlers, sizeof(s_user_interupt_handlers), 0xff);
}

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

void generic_irq_handler()
{   
    uint32_t irq_source = mmio_read(CORE_0_IRQ_SOURCE); // TODO check if is core other then zero
    interupt_active = true;
    int irq = -1;

    if (irq_source & (1 << 1))     // Handle physical timer
    {
        irq = 30;
    }
    else                            // Handle GPU interupt
    {
        uint64_t irq_raw = mmio_read(IRQ_PENDING_1) | (((uint64_t)mmio_read(IRQ_PENDING_2)) << 32);
        
        if (irq_raw)
        {
            irq = __builtin_ctzll(irq_raw);
        }
    }

    if (irq == 30)
        keypad_poll_from_timer();
    else if (irq == 57)
        keypad_uart_interupt_handler();
    else if (irq == 52)
        gpio_interupt_handler_function();
    else if (irq == -1)
    {
        printf("\n!============================!\n");
        printf("\nError: unable to find irq number\n");
        printf("CORE_0_IRQ_SOURCE: %x\n", irq_source);
        printf("IRQ_PENDING_1: %x\n", mmio_read(IRQ_PENDING_1));
        printf("IRQ_PENDING_1: %x\n\n", mmio_read(IRQ_PENDING_2));
        printf("\n!============================!\n");
    }
    else
    {
        // TODO handler

        printf("\n!============================!\nUnkown IRQ %d\n", irq);
        kernel_panic();
    }

    interupt_active = false;

    event_handler_on_interupt_end();
}

void register_user_interupt_handler(USER_INTERUPT_HANDLER handler, int id)
{
    if (id < 0 || id > POINTER_MAX)
    {
        printf("Error: Invaild user interupt ID: %d\n", id);
        return;
    }

    s_user_interupt_handlers[id] = handler;
}

void remove_user_interupt_handler(int id)
{  
    register_user_interupt_handler((USER_INTERUPT_HANDLER)POINTER_MAX, id);
}

void trigger_user_interupt_handler(int id)
{
    if (id < 0 || id > POINTER_MAX)
    {
        printf("Error: Invaild user interupt ID: %d\n", id);
        return;
    }

    if (s_user_interupt_handlers[id] == (USER_INTERUPT_HANDLER)POINTER_MAX)
    {
        printf("Error: Invaild user interupt: %d", id);
        return;
    }

    execute_function_as_user_program(get_active_user_program(), (USER_FUNCTION)s_user_interupt_handlers[id]);
}