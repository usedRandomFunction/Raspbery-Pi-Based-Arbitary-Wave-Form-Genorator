#include "lib/timing.h"

#include "io/memoryMappedIO.h"

void wait_cycles(size_t cycles)
{
    if (cycles == 0)
        return;

    while (cycles--)
    {
        asm volatile ("nop");
    }
    
}

// Function from https://github.com/bztsrc/raspi3-tutorial/blob/master/07_delays/delays.c
void delay_microseconds(size_t microseconds)
{
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // calculate required count increase
    unsigned long i=((f/1000)*microseconds)/1000;
    // loop while counter increase is less than i
    do{asm volatile ("mrs %0, cntpct_el0" : "=r"(r));}while(r-t<i);
}

void delay_milliseconds(size_t milliseconds)
{
    delay_microseconds(milliseconds * 1000);
}

uint32_t get_timer_frequency()
{
    uint32_t reg;

    asm volatile ("mrs %0, cntfrq_el0" : "=r"(reg));

    return reg;
}

uint64_t get_timer_count()
{
    uint64_t reg;

    asm volatile ("mrs %0, cntpct_el0" : "=r"(reg));

    return reg;
}

void enable_timer_interrupt()
{
    mmio_write(CORE_0_TIMER_IRQ_CTRL, 1 << 1);
}

void set_timer_interrupt(uint32_t microseconds)
{   
    uint32_t freqency = get_timer_frequency();
    float timer_value = freqency * ((float)microseconds / 1000 / 1000);
    

    uint32_t register_value = (uint32_t)timer_value;

    asm volatile ("msr cntp_tval_el0, %0" : : "r"(register_value));

    register_value = 1;
    asm volatile ("msr cntp_ctl_el0, %0" : : "r"(register_value));
}

void dissable_timer_interrupt()
{
    mmio_write_bitwise_and(CORE_0_TIMER_IRQ_CTRL, ~(1 << 1));
    asm volatile ("msr cntp_ctl_el0, xzr");
}