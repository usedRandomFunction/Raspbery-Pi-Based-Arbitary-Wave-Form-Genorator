#ifndef GPIO_H
#define GPIO_H

#include "memoryMappedIO.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Use the handle gpio interupts
// @param pin, The pin number used
// @return Nothing
typedef void (*GPIO_INTERUPT_HANDLER)(int);

// Enables irq 52 and clears the handler pointers
void initialize_gpio_interupts();

// Selects the functionallity of the given GPIO pin
// @param pin The pin to set
// @param function the function of the pin
void gpio_function_select(uint_fast8_t pin, uint_fast32_t function);

// Used to ennable / toggle gpio interupts
// @param pin The pin to enable interupts on
// @param handler The handler function
// @param rising_edge Whether or not to trigger on the rising edge, using the system clock
// @param falling_edge Whether or not to trigger on the falling edge, using the system clock
// @param high_level Whether or not to trigger on high level pins
// @param low_level Whether or not to trigger on low level pins
// @param async_rising Whether or not to trigger on the rising edge, not dependent on the system clock
// @param async_falling Whether or not to trigger on the falling edge, not dependent on the system clock
void gpio_enable_pin_interupt(int pin, GPIO_INTERUPT_HANDLER handler,
    bool rising_edge, bool falling_edge, bool high_level, bool low_level, bool async_rising, bool async_falling);

// Dissables all interupts on a pin
// @param pin The pin to dissable
// @note Same as calling gpio_enable_pin_interupt(pin, NULL, false, false, false, false, false, false);
void gpio_disable_pin_interupt(int pin);

// Used to handle irq 52, and run the respective function
// @note Only to be called by generic_irq_handler()
void gpio_interupt_handler_function();


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