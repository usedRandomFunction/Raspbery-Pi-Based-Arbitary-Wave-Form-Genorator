#include "lib/random.h"

#include "io/memoryMappedIO.h"

void initialize_random()
{
    mmio_write(RNG_STATUS, 0x40000);
    mmio_write_bitwise_or(RNG_INT_MASK, 1);
    mmio_write_bitwise_or(RNG_CTRL, 1);
}

uint32_t random(uint32_t min, uint32_t max)
{
    while(!(mmio_read(RNG_STATUS) >> 24)) 
    {
        asm volatile("nop");
    }
    
    return mmio_read(RNG_DATA) % (max-min) + min;
}