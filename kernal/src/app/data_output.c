#include "app/data_output.h"
#include "lib/math.h"
#include "io/gpio.h"

#include "io/uart.h"

void write_buffer_one_channle_8_bit_samples(uint64_t* buffer_start, uint8_t sample, uint64_t offset)
{

    uint64_t buffer_entry = buffer_start[offset / 8];
    uint64_t mask = 0xFF << 18;
    uint8_t bit_offset = (offset & 7) * 8;// offset & 7 is the same as offset % 8
    
    uint64_t data = 0;
    // DO NOT EDIT 
    // These values are based of the traces and need outputs of the shift registers 
    // So no touchy, i just spend hours debuging these (And it turned out to be math.h...)
    data |= (sample & (1 << 4)) >> 4 << 0;
    data |= (sample & (1 << 7)) >> 7 << 1;
    data |= (sample & (1 << 3)) >> 3 << 2;
    data |= (sample & (1 << 6)) >> 6 << 3;
    data |= (sample & (1 << 2)) >> 2 << 4;
    data |= (sample & (1 << 5)) >> 5 << 5;
    data |= (sample & (1 << 0)) >> 0 << 6;
    data |= (sample & (1 << 1)) >> 1 << 7;

    data <<= 18;
    
    rotate_left(mask, mask, bit_offset);
    rotate_left(data, data, bit_offset);
    
    buffer_entry &= ~mask;
    buffer_entry |= data; 


    buffer_start[offset / 8] = buffer_entry;
}

#define shift_register_out() *clr_address = clock_mask | (data_mask & ~(current_data)); *set_address = data_mask & (current_data); *set_address = clock_mask;
#define latch() *set_address = latch_pin; *clr_address = latch_pin;

void write_one_channel_8_bit_samples(uint64_t* buffer_start, uint64_t* buffer_end)
{
    const uint32_t latch_pin = 1 << 16;
    const uint32_t clock_mask = 1 << 17;
    const uint32_t data_mask = 1 << 18 | 1 << 19;

    uint64_t* ptr = buffer_start;
    uint64_t current_data = 0;

    volatile uint32_t* set_address = get_mmio_pointer(GPSET0);
    volatile uint32_t* clr_address = get_mmio_pointer(GPCLR0);

    while (1)
    {
        __builtin_prefetch(ptr);

        // Expaned for loop
        #pragma region Expaned for loop 1
        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();


        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();


        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();


        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();


        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();


        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();


        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();


        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        shift_register_out();
        rotate_right(current_data, current_data, 2);

        latch();
        #pragma endregion

        current_data = *ptr;
        ptr++;

        if (ptr == buffer_end)
            ptr = buffer_start;
    }
}