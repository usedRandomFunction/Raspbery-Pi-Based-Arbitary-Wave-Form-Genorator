#include "buffer_management.h"

#include "common/program_managment.h"
#include "common/output.h"
#include "raw.h"

// Calculates the minimum frame size that can store the enabled channels
// @param enabled_channels Selects what channels are enabled, the value of the Nth bit represents the value for the Nth channel
// @returns Minimum frame size for given channels
static int s_calculate_minimum_frame_size(int enabled_channels);

// Calculates the frame offset required for the minimum sized buffer, required by the enabled channels
// @param enabled_channels Selects what channels are enabled, the value of the Nth bit represents the value for the Nth channel
// @returns Frame offset in bits
static int s_calculate_frame_offset(int enabled_channels);

// Handles buffer allocation (If applicable), and sets the values in the buffer struct occurdingly 
// Used to set enabled channels, resolution and number of sample for a given buffer
// @param enabled_channels Selects what channels are enabled, the value of the Nth bit represents the value for the Nth channel
// @param resolution Resolution in bits
// @param number_of_samples Total number of samples, note that this number is not the total of all channels, but the number of number of samples per channel
// @param buffer Buffer ot work with
// @return Less then 0 if a error occured, > 1 if success.
static int s_prepair_buffer(int enabled_channels, int resolution, size_t number_of_samples, output_buffer* buffer);


int setup_output_buffer(int enabled_channels, int resolution, size_t number_of_samples, output_buffer* buffer)
{
    if (buffer->resolution != resolution || buffer->number_of_samples != number_of_samples)
        return s_prepair_buffer(enabled_channels, resolution, number_of_samples, buffer);

    const int new_minimum_frame_size = s_calculate_minimum_frame_size(enabled_channels);
    const int new_frame_offset = s_calculate_frame_offset(enabled_channels);

    if (new_minimum_frame_size != buffer->frame_size || new_frame_offset != buffer->frame_offset)
        return s_prepair_buffer(enabled_channels, resolution, number_of_samples, buffer);

    buffer->enabled_channels = enabled_channels;

    return 0;
}


void write_to_output_buffer(float voltage, int channel, size_t index, output_buffer* buffer)
{
    int value = voltage_to_dac_value(voltage, buffer->resolution);


    write_dac_buffer_entry(buffer->buffer, value, index, channel, buffer->resolution, buffer->frame_size, buffer->frame_offset);
}

void start_output_from_buffer(output_buffer* buffer, bool dont_loop)
{
    int flags = get_standered_dac_flags_from_buffer(buffer);

    if (dont_loop)
        flags |= DAC_OUTPUT_FLAGS_DONT_LOOP;

    dac_output_start(buffer->buffer, buffer->number_of_samples, flags);
}

int get_standered_dac_flags_from_buffer(output_buffer* buffer)
{
    int flags = 0;


    // Since in the flags varible for the enabled channels is 64 for ch1 then 128 for ch2... 
    // and the buffer struct uses Nth bit repesnents Nth bit, we can get away with timesing by CH1
    const int enabled_channels_bits = (buffer->enabled_channels & 0b1111) * DAC_OUTPUT_FLAGS_CH1_ENABLED;
    flags |= enabled_channels_bits;

    // Note this value only really exists in the flags, so they can fully describe the buffer
    // but it is not used by dac_output_start rn but is included for completeness 
    const int frame_start_channel = (buffer->frame_offset - 18) / 2;
    flags |= (frame_start_channel & 0b11) << 4;

    // This one is just a bit to small to use a switch case
    if (buffer->resolution == 2)
        flags |= DAC_OUTPUT_FLAGS_FRAME_SIZE_2_BITS;
    else if (buffer->resolution == 4)
        flags |= DAC_OUTPUT_FLAGS_FRAME_SIZE_4_BITS;
    else if (buffer->resolution == 8)
        flags |= DAC_OUTPUT_FLAGS_FRAME_SIZE_8_BITS;


    return flags;
}

static int s_calculate_minimum_frame_size(int enabled_channels)
{
    enabled_channels &= 0b111;
    if (enabled_channels == 0)
        return 0;               // Handle invalid values

    // Bitshift untill it started with an enabled channel or
    while (!(enabled_channels & 0b1))
    {
        enabled_channels >>= 1;
    }


    int i = 0;

    while (enabled_channels)
    {
        enabled_channels >>=1;
        i++;
    }
    
    return i * 2;
}

static int s_calculate_frame_offset(int enabled_channels)
{
    enabled_channels &= 0b111;
    if (enabled_channels == 0)
        return 0;               // Handle invalid values

    int i = 0;

    while (!(enabled_channels & 0b1))
    {
        enabled_channels >>= 1;
        i++;
    }

    return 16 + i * 2;
}

static int s_prepair_buffer(int enabled_channels, int resolution, size_t number_of_samples, output_buffer* buffer)
{
    const int frame_size = s_calculate_minimum_frame_size(enabled_channels);
    const int frame_offset = s_calculate_frame_offset(enabled_channels);
    const int frames_per_sample = resolution / 2;
    
    buffer->number_of_samples = number_of_samples;
    buffer->enabled_channels = enabled_channels;
    buffer->frame_offset = frame_offset;
    buffer->frame_size = frame_size;
    buffer->resolution = resolution;
    // buffer->buffer_size

    const size_t required_bits = frame_size * frames_per_sample * number_of_samples;
    const size_t minimum_size_bytes = required_bits / 8 + (required_bits % 8 ? 1 : 0);  // Devide round up
    const size_t true_size = vmemmap(buffer->buffer, minimum_size_bytes, VMEMMAP_WRITABILITY);

    if (true_size < minimum_size_bytes)
        return -1;
    
    buffer->buffer_size = true_size;

    return 1;
}