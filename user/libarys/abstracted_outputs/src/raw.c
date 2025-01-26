#include "raw.h"

#include "internal/config.h"
#include "common/math.h"

#include <stdint.h>

// Takes the given value and remixes the bits so they match with how
// tehy are supost to be output by the gpio as described by doc/ouptuts.md
// @param value the value to remix
// @param resolution DAC resolution used
// @return Output values stored as LSB, MSB, LSB, MSB, ... first in number are to be transimitted first
static uint64_t s_remix_value_to_frame_bits(int value, int resolution);

int voltage_to_dac_value(float voltage, int resolution)
{
    voltage = clamp(voltage, DAC_VOLTAGE_MAX, DAC_VOLTAGE_MIN);
    voltage /= (float)DAC_VOLTAGE_MAX;

    float value = voltage * (float)((1 << resolution) - 1);

    return (int)value;
}


void write_dac_buffer_entry(void* buffer_start, int value, size_t index, int channel, int resolution, int frame_size, int frame_offset)
{
    value = s_remix_value_to_frame_bits(value, resolution);

    uint64_t bit_mask = UINT64_MAX;
    uint64_t new_bits = 0;

    for (int i = 0; i < resolution / 2; i++)
    {
        new_bits |= (value & 0b11) << (i * frame_size);
        bit_mask &= ~(0b11 << i * frame_size);
        value >>= 2;
    }

    const int frames_per_value = resolution / 2;
    const int frames_per_u64 = 64 / frame_size;
    const int values_per_u64 = frames_per_u64 / frames_per_value;
    const int buffer_index = index / values_per_u64;
    const int entry_offset = index % values_per_u64;
    const int index_offest = entry_offset * frames_per_value * frame_size;
    const int base_offset = frame_offset + channel * 2;

    rotate_left(new_bits, new_bits, base_offset + index_offest);
    rotate_left(bit_mask, bit_mask, base_offset + index_offest);


    uint64_t* buffer = (uint64_t*)buffer_start;
    buffer[buffer_index] &= bit_mask;
    buffer[buffer_index] |= new_bits;
}

// Gets the nth bit and shifts it so it is now bit 0
#define get_nth_bit_shifted(value, n) (value & (1 << n)) >> n

uint64_t s_remix_value_to_frame_bits(int value, int resolution)
{
     uint64_t data = 0;
    // DO NOT EDIT 
    // These values are based of the traces and need outputs of the shift registers 
    // So no touchy, i just spend hours debuging these (And it turned out to be math.h...)


    data |= get_nth_bit_shifted(value, 0) << 0;
    data |= get_nth_bit_shifted(value, 1) << 1;

    if (resolution == 2)
        return data;

    data <<= 6;
    
    data |= get_nth_bit_shifted(value, 4) << 0;
    data |= get_nth_bit_shifted(value, 7) << 1;
    data |= get_nth_bit_shifted(value, 3) << 2;
    data |= get_nth_bit_shifted(value, 6) << 3;
    data |= get_nth_bit_shifted(value, 2) << 4;
    data |= get_nth_bit_shifted(value, 5) << 5;

    if (resolution == 8)
        return data;

    data <<= 8;

    data |= get_nth_bit_shifted(value, 11) << 0;
    data |= get_nth_bit_shifted(value, 15) << 1;
    data |= get_nth_bit_shifted(value, 10) << 2;
    data |= get_nth_bit_shifted(value, 14) << 3;
    data |= get_nth_bit_shifted(value, 9)  << 4;
    data |= get_nth_bit_shifted(value, 13) << 5;
    data |= get_nth_bit_shifted(value, 8)  << 6;
    data |= get_nth_bit_shifted(value, 12) << 7;

    // But what if its not 2, 8 or 16 bit
    // To bad so sad, undefined in docs, 
    // so I dont need to do anything

    return data;
}