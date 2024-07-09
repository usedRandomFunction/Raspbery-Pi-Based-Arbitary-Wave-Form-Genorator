#ifndef DATA_OUTPUT_H
#define DATA_OUTPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Adds an entry to the buffer
void write_buffer_one_channle_8_bit_samples(uint64_t* buffer_start, uint8_t sample, uint64_t offset);

// Continusly write the buffer to the shift registers in a loop
void write_one_channel_8_bit_samples(uint64_t* buffer_start, uint64_t* buffer_end);



#ifdef __cplusplus
}
#endif


#endif