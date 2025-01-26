#ifndef ABSTRACTED_OUTPUTS_RAW_H
#define ABSTRACTED_OUTPUTS_RAW_H

#include <stddef.h>

// Takes a voltage, and turns it into the corrisponding DAC value for a given resolution
// @param voltage Voltage to convert
// @param resolution The DAC resolution to use
// @return Value for DAC
int voltage_to_dac_value(float voltage, int resolution);

// Writes a buffer entry given the channel, resolution, frame size, and frame offset
// @param buffer_start Pointer to the start of the value buffer
// @param value Output value for the DAC
// @param index Possition in value buffer
// @param channel The channel we are writting to (Note this value starts at zero not 1)
// @param resolution The resolution in bits, we are working with
// @param frame_size The size of a output frame in bits (see doc/ouptuts.md)
// @param frame_offset The offest, form the start of a uint64_t to the first frame (18 for ch1, 20 for 2, 22 for 3, and for 24 for 4, (First ch in frame))
void write_dac_buffer_entry(void* buffer_start, int value, size_t index, int channel, int resolution, int frame_size, int frame_offset);

#endif