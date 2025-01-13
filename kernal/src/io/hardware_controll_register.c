#include "io/hardware_controll_register.h"
#include "run_time_kernal_config.h"
#include "lib/memory.h"
#include "lib/timing.h"
#include "io/gpio.h"
#include "io/spi.h"

#define controll_data_latch_pin 5

uint8_t hardware_controll_register[HARDWARE_CONTROLL_REGISTER_SIZE_BYTES];

uint8_t* hardware_controll_register_keypad_controll_byte = hardware_controll_register;
uint8_t* hardware_controll_register_relay_byte = hardware_controll_register + 1;

void initialize_hardware_controll_register()
{
    if (is_running_in_qemu)     // QEMU halts if we try to continue form here
        return;

    memclr(hardware_controll_register, HARDWARE_CONTROLL_REGISTER_SIZE_BYTES);
    
    gpio_function_select(controll_data_latch_pin, GPFSEL_Output);
    gpio_clear(controll_data_latch_pin);

    initialize_spi0(spi_clock_frequency);


    hardware_controll_register_write();
}

void hardware_controll_register_write()
{
    hardware_controll_register_write_read(NULL);
}

void hardware_controll_register_write_read(void* recive_buffer)
{
    if (is_running_in_qemu)     // QEMU halts if we try to continue form here
        return;

    spi0_write_read(3, hardware_controll_register, recive_buffer, HARDWARE_CONTROLL_REGISTER_SIZE_BYTES);

    gpio_set(controll_data_latch_pin);
    wait_cycles(hardware_controll_register_latch_delay);
    gpio_clear(controll_data_latch_pin);
}