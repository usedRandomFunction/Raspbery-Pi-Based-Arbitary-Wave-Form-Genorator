#ifndef HARDWARE_CONTROLL_REGISTER_H
#define HARDWARE_CONTROLL_REGISTER_H

#include <stdint.h>

#define HARDWARE_CONTROLL_REGISTER_SIZE_BYTES 2

extern uint8_t hardware_controll_register[];

extern uint8_t* hardware_controll_register_keypad_controll_byte;
extern uint8_t* hardware_controll_register_relay_byte;

// Initializes SPI and writes zeros to the controll register
void initialize_hardware_controll_register();

// Writes the hardware_controll_register[] buffer
// to the register its self.
void hardware_controll_register_write();

// Writes the hardware_controll_register[] buffer
// to the register its self, at the same time reading 
// bytes of the SPI
// @param recive_buffer buffer to read into, must be at least HARDWARE_CONTROLL_REGISTER_SIZE_BYTES bytes
void hardware_controll_register_write_read(void* recive_buffer);

#endif