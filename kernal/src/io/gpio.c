#include "io/gpio.h"

#include "lib/interrupts.h"
#include "lib/memory.h"
#include "io/printf.h"


static GPIO_INTERUPT_HANDLER s_interupt_hanlders[54];

void initialize_gpio_interupts()
{
    memclr(s_interupt_hanlders, sizeof(s_interupt_hanlders));

    // Clear all events and dissable all interupts

    mmio_write(GPREN0, 0);
    mmio_write(GPREN1, 0);
    mmio_write(GPFEN0, 0);
    mmio_write(GPFEN1, 0);
    mmio_write(GPHEN0, 0);
    mmio_write(GPHEN1, 0);
    mmio_write(GPLEN0, 0);
    mmio_write(GPLEN1, 0);
    mmio_write(GPAREN0, 0);
    mmio_write(GPAREN1, 0);
    mmio_write(GPAFEN0, 0);
    mmio_write(GPAFEN1, 0);
    mmio_write(GPEDS0, 0xFFFFFFFF);
    mmio_write(GPEDS1, 0xFFFFFFFF);

    enable_irq(52);
}

void gpio_function_select(uint_fast8_t pin, uint_fast32_t function)
{
    size_t registerAddress;
    uint_fast8_t offset;

    switch (pin)
    {
    case 0: registerAddress = GPFSEL0; offset = 3 * 0; break;
    case 1: registerAddress = GPFSEL0; offset = 3 * 1; break;
    case 2: registerAddress = GPFSEL0; offset = 3 * 2; break;
    case 3: registerAddress = GPFSEL0; offset = 3 * 3; break;
    case 4: registerAddress = GPFSEL0; offset = 3 * 4; break;
    case 5: registerAddress = GPFSEL0; offset = 3 * 5; break;
    case 6: registerAddress = GPFSEL0; offset = 3 * 6; break;
    case 7: registerAddress = GPFSEL0; offset = 3 * 7; break;
    case 8: registerAddress = GPFSEL0; offset = 3 * 8; break;
    case 9: registerAddress = GPFSEL0; offset = 3 * 9; break;
    
    case 10: registerAddress = GPFSEL1; offset = 3 * 0; break;
    case 11: registerAddress = GPFSEL1; offset = 3 * 1; break;
    case 12: registerAddress = GPFSEL1; offset = 3 * 2; break;
    case 13: registerAddress = GPFSEL1; offset = 3 * 3; break;
    case 14: registerAddress = GPFSEL1; offset = 3 * 4; break;
    case 15: registerAddress = GPFSEL1; offset = 3 * 5; break;
    case 16: registerAddress = GPFSEL1; offset = 3 * 6; break;
    case 17: registerAddress = GPFSEL1; offset = 3 * 7; break;
    case 18: registerAddress = GPFSEL1; offset = 3 * 8; break;
    case 19: registerAddress = GPFSEL1; offset = 3 * 9; break;

    case 20: registerAddress = GPFSEL2; offset = 3 * 0; break;
    case 21: registerAddress = GPFSEL2; offset = 3 * 1; break;
    case 22: registerAddress = GPFSEL2; offset = 3 * 2; break;
    case 23: registerAddress = GPFSEL2; offset = 3 * 3; break;
    case 24: registerAddress = GPFSEL2; offset = 3 * 4; break;
    case 25: registerAddress = GPFSEL2; offset = 3 * 5; break;
    case 26: registerAddress = GPFSEL2; offset = 3 * 6; break;
    case 27: registerAddress = GPFSEL2; offset = 3 * 7; break;
    case 28: registerAddress = GPFSEL2; offset = 3 * 8; break;
    case 29: registerAddress = GPFSEL2; offset = 3 * 9; break;

    case 30: registerAddress = GPFSEL3; offset = 3 * 0; break;
    case 31: registerAddress = GPFSEL3; offset = 3 * 1; break;
    case 32: registerAddress = GPFSEL3; offset = 3 * 2; break;
    case 33: registerAddress = GPFSEL3; offset = 3 * 3; break;
    case 34: registerAddress = GPFSEL3; offset = 3 * 4; break;
    case 35: registerAddress = GPFSEL3; offset = 3 * 5; break;
    case 36: registerAddress = GPFSEL3; offset = 3 * 6; break;
    case 37: registerAddress = GPFSEL3; offset = 3 * 7; break;
    case 38: registerAddress = GPFSEL3; offset = 3 * 8; break;
    case 39: registerAddress = GPFSEL3; offset = 3 * 9; break;

    case 40: registerAddress = GPFSEL4; offset = 3 * 0; break;
    case 41: registerAddress = GPFSEL4; offset = 3 * 1; break;
    case 42: registerAddress = GPFSEL4; offset = 3 * 2; break;
    case 43: registerAddress = GPFSEL4; offset = 3 * 3; break;
    case 44: registerAddress = GPFSEL4; offset = 3 * 4; break;
    case 45: registerAddress = GPFSEL4; offset = 3 * 5; break;
    case 46: registerAddress = GPFSEL4; offset = 3 * 6; break;
    case 47: registerAddress = GPFSEL4; offset = 3 * 7; break;
    case 48: registerAddress = GPFSEL4; offset = 3 * 8; break;
    case 49: registerAddress = GPFSEL4; offset = 3 * 9; break;
    
    case 50: registerAddress = GPFSEL5; offset = 3 * 0; break;
    case 51: registerAddress = GPFSEL5; offset = 3 * 1; break;
    case 52: registerAddress = GPFSEL5; offset = 3 * 2; break;
    case 53: registerAddress = GPFSEL5; offset = 3 * 3; break;

    default: // If this pin doesn't exist, do nothing
        return;
    }

    mmio_write_offset_of_size(registerAddress, function, offset, 3);
}

void gpio_enable_pin_interupt(int pin, GPIO_INTERUPT_HANDLER handler,
    bool rising_edge, bool falling_edge, bool high_level, bool low_level, bool async_rising, bool async_falling)
{
    if (pin < 0 || pin > 53)   // TODO maby a exception here
        return;

    gpio_function_select(pin, GPFSEL_Input);

    int register_offset;
    int bit_offset;

    if (pin < 32)
    {
        register_offset = 0;
        bit_offset = pin;
    }
    else
    {
        register_offset = 4;
        bit_offset = pin - 32;
    }

    uint32_t enable_bit = 1 << bit_offset;
    uint32_t dissable_mask = ~enable_bit;
    s_interupt_hanlders[pin] = handler;

    if (rising_edge)
        mmio_write_bitwise_or(GPREN0 + register_offset, enable_bit);
    else
        mmio_write_bitwise_and(GPREN0 + register_offset, dissable_mask);

    if (falling_edge)
        mmio_write_bitwise_or(GPFEN0 + register_offset, enable_bit);
    else
        mmio_write_bitwise_and(GPFEN0 + register_offset, dissable_mask);

    if (high_level)
        mmio_write_bitwise_or(GPHEN0 + register_offset, enable_bit);
    else
        mmio_write_bitwise_and(GPHEN0 + register_offset, dissable_mask);

    if (low_level)
        mmio_write_bitwise_or(GPLEN0 + register_offset, enable_bit);
    else
        mmio_write_bitwise_and(GPLEN0 + register_offset, dissable_mask);

    if (async_rising)
        mmio_write_bitwise_or(GPAREN0 + register_offset, enable_bit);
    else
        mmio_write_bitwise_and(GPAREN0 + register_offset, dissable_mask);

    if (async_falling)
        mmio_write_bitwise_or(GPAFEN0 + register_offset, enable_bit);
    else
        mmio_write_bitwise_and(GPAFEN0 + register_offset, dissable_mask);
}

void gpio_disable_pin_interupt(int pin)
{
    gpio_enable_pin_interupt(pin, NULL, false, false, false, false, false, false);
}

void gpio_interupt_handler_function()
{
    disable_irq(52);

    uint64_t status_registers = (uint64_t)mmio_read(GPEDS0) |  (((uint64_t)mmio_read(GPEDS1)) << 32);

    for (int i = 0; i < 54; i++)
    {
        if ((status_registers & (1ULL << i)) == 0)
            continue;

        if (s_interupt_hanlders[i] == NULL)
        {
            printf("Error: Unkown GPIO interupt: %d\n", i);

            continue;
        }

        s_interupt_hanlders[i](i);
    }

    mmio_write(GPEDS0, (uint32_t)status_registers);
    mmio_write(GPEDS1, (uint32_t)(status_registers >> 32));

    enable_irq(52);
}