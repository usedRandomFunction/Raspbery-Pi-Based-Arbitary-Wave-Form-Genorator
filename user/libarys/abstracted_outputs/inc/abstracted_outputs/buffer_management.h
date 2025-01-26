#ifndef BUFFER_MANAGEMENT_H
#define BUFFER_MANAGEMENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct output_buffer
{
    int resolution;
    int frame_offset;
    int frame_size;
    int enabled_channels; // For this varible the value of the Nth bit represents the value for the Nth channel


    size_t number_of_samples;
    size_t buffer_size;
    void* buffer;
};


typedef struct output_buffer output_buffer;


// Used to set enabled channels, resolution and number of sample for a given buffer
// @param enabled_channels Selects what channels are enabled, the value of the Nth bit represents the value for the Nth channel
// @param resolution Resolution in bits
// @param number_of_samples Total number of samples, note that this number is not the total of all channels, but the number of number of samples per channel
// @param buffer Buffer ot work with
// @return Less then 0 if a error occured, 0 if success, and the old values buffer doesn't need to be remade, > 1 if success, but all values need to be remade
int setup_output_buffer(int enabled_channels, int resolution, size_t number_of_samples, output_buffer* buffer);

// Writes the voltage to the given channel at the given index
// @param voltage The target output voltage for this sample
// @param channel Which channel we are outputing to
// @param index Which sample are we writing to
// @param buffer Which buffer to write into
void write_to_output_buffer(float voltage, int channel, size_t index, output_buffer* buffer);

// Starts the DAC output for the given buffer
// @param buffer Buffer to output
// @param dont_loop If set to true, the buffer wont loop, but insted will just end, when it reaches the last sample
// @note You still need to call dac_output_end to stop the output
void start_output_from_buffer(output_buffer* buffer, bool dont_loop);

// Create flags as defined in project_specific_syscalls.md, from the given buffer
// @param buffer the output_buffer struct to create flags for
// @return dac_output_start flags compliant to project_specific_syscalls.md
int get_standered_dac_flags_from_buffer(output_buffer* buffer);

#endif