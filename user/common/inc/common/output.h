#ifndef OUTPUT_H
#define OUTPUT_H

#include <stddef.h>

// When called the calling thread will be used by the kernal to
// output data to the DACs, in the method described in outputs.md.
// @param buffer_start Pointer to the value buffer
// @param n The number of values in the buffer (For each channel not all of them added)
// @param flags Flags (DAC_OUTPUT_FLAGS)
// @return zero if exited with dac_output_end or reached the end of the buffer, and non-zero on failer, the first two bits of return value state what channel is incompitible. If Bit 3 is set then the error is not specific to the channels.
int dac_output_start(void* buffer_start, size_t n, int flags);

// When called stops the DAC output on all threads.
// @note This function is designed to be called at the end of a interut and will take over controll. If dac_output_start was not called this function will simply return 
void dac_output_end();

// Sets / gets the resolution of all the DACs
// @param resolution The resolution in bits suported resolutions are `2`, `8`, `16` if `-1` is given it will simply return the current resolution
// @return The resolution at the end of the operation
int dac_resolution(int resolution);

// Enables, dissables or checks the output buffers on the given channel
// @param channel The channel to check `1`, `2`, `3` or `4`
// @param state `1` Represents enabled `0`, represents dissabled, while `-1` returns the current state
// @return The state at teh end of the operation
int dac_channel_buffering(int channel, int state);

// Gets the sample rate Returns the sample rate, in hertz, at a given resolution
// @param resolution The resolution in bits suported resolutions are `2`, `8`, `16` if `-1` is given it will simply use the current resolution
// @return The sample rate in hertz or 0 if the resolution is not supported
int dac_get_sample_rate(int resolution);

// States if a given channel supports a given config
// @param channel The channel to check `1`, `2`, `3` or `4`
// @param resolution The resolution in bits suported resolutions are `2`, `8`, `16` if `-1` is given it will simply use the current resolution
// @param buffered Wether or not the output is buffered  `1` Represents enabled `0`, represents dissabled, while `-1` uses the current state
// @return `0` If config is suported, if the buffering state is not bit 0 is set, if the resolution is not suported bit 2 is set
int dac_channel_supports_config(int channel, int resolution, int buffered);



enum 
{
    DAC_OUTPUT_FLAGS_FRAME_SIZE_2_BITS  =   0,      /*Sets details about the output frame see outputs.md*/
    DAC_OUTPUT_FLAGS_FRAME_SIZE_4_BITS  =   1,      /*Sets details about the output frame see outputs.md*/
    DAC_OUTPUT_FLAGS_FRAME_SIZE_8_BITS  =   2,      /*Sets details about the output frame see outputs.md*/
    DAC_OUTPUT_FLAGS_FRAME_START_CH_1   =   0,      /*Sets details about the output frame see outputs.md*/
    DAC_OUTPUT_FLAGS_FRAME_START_CH_2   =   16,     /*Sets details about the output frame see outputs.md*/
    DAC_OUTPUT_FLAGS_FRAME_START_CH_3   =   32,     /*Sets details about the output frame see outputs.md*/
    DAC_OUTPUT_FLAGS_FRAME_START_CH_4   =   48,     /*Sets details about the output frame see outputs.md*/
    DAC_OUTPUT_FLAGS_CH1_ENABLED        =   64,     /*If set enables channel 1*/
    DAC_OUTPUT_FLAGS_CH2_ENABLED        =   128,    /*If set enables channel 2*/
    DAC_OUTPUT_FLAGS_CH3_ENABLED        =   256,    /*If set enables channel 3*/
    DAC_OUTPUT_FLAGS_CH4_ENABLED        =   512,    /*If set enables channel 4*/
    DAC_OUTPUT_FLAGS_DONT_LOOP          =   1024,   /*If set the output will stop when the end of buffer is reached, insted of looping.*/
};

#endif