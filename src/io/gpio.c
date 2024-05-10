#include "io/gpio.h"

void gpio_function_select(uint_fast8_t pin, uint_fast32_t functionSelect)
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

    mmio_write_offset_of_size(registerAddress, functionSelect, offset, 3);
}
