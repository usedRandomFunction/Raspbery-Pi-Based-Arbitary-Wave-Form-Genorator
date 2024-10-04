#ifndef GPIO_H
#define GPIO_H

#include "memoryMappedIO.h"

#include <stddef.h>
#include <stdint.h>



// Selects the functionallity of the given GPIO pin
// @param pin The pin to set
// @param function the function of the pin
void gpio_function_select(uint_fast8_t pin, uint_fast32_t function);

#pragma GCC diagnostic ignored "-Wshift-count-negative"
#define gpio_set(pin) if (pin < 32) { mmio_write(GPSET0, 1 << pin); } else { mmio_write(GPSET1, 1 << (pin - 32)); }
#pragma GCC diagnostic ignored "-Wshift-count-negative"
#define gpio_clear(pin) if (pin < 32) { mmio_write(GPCLR0, 1 << pin); } else { mmio_write(GPCLR1, 1 << (pin - 32)); }
#pragma GCC diagnostic ignored "-Wshift-count-negative"
#define gpio_level(pin) (pin < 32 ? mmio_read(GPLEV0, 1 << pin) : mmio_read(GPLEV1, 1 << (pin - 32)))
#pragma GCC diagnostic ignored "-Wshift-count-negative"
#define gpio_write(pin, value) if (value) { gpio_set(pin); } else { gpio_clear(pin); }

enum
{
    GPFSEL_Input = 0b000,
    GPFSEL_Output = 0b001,
    GPFSEL_Alternate0 = 0b100,
    GPFSEL_Alternate1 = 0b101,
    GPFSEL_Alternate2 = 0b110,
    GPFSEL_Alternate3 = 0b111,
    GPFSEL_Alternate4 = 0b011,
    GPFSEL_Alternate5 = 0b010,
};



#endif