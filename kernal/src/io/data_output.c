#include "io/data_output.h"

#include "io/hardware_controll_register.h"
#include "lib/config_file.h"
#include "lib/memory.h"
#include "io/printf.h"
#include "lib/math.h"
#include "io/gpio.h"


struct dac_info
{   // Config settings here
    bool supports_resolution_16_bit;
    bool supports_resolution_8_bit;
    bool supports_resolution_2_bit;
    bool supports_state_unbuffered;
    bool supports_state_buffered;

    // Run time data here
    bool is_buffered;
};

typedef struct dac_info dac_info;

static int s_current_resolution = 0;
static int s_sample_rate_16_bit = 0;
static int s_sample_rate_8_bit = 0;
static int s_sample_rate_2_bit = 0;
static dac_info s_dacs[4];

// Does as the name says
// @param flags Flags from DAC_OUTPUT_FLAGS enum
// @return the data mask form the flags
static uint32_t s_create_data_mask_from_flags(int flags);

// Loads the sample rates at diffrent resolutions
static void s_load_sample_rates_from_file();

// Loads config values for the outputs from their respective files
static void s_load_output_configs_from_file();

// Loads the config values for a given output, from the given file
// @param output The dac_info struct to store the values in
// @param path The path to the config file
static void s_load_output_config_from_file(dac_info* output, const char* path);

// Checks if the enabled channels from flags, support the current dac states.
// @param flags Flags from DAC_OUTPUT_FLAGS enum
// @return Error bits as defined for dac_output_start
static int s_check_if_flag_channels_support_current_state(int flags);

// Sets the defult resolution if the DAC based of what channels are supported
static void s_set_defult_resolution();

// Sets the defult buffering states for the outputs, base of what is supported
static void s_set_defult_buffering_states();

#define shift_register_out() *clr_address = clock_mask | (data_mask & ~(current_data)); *set_address = data_mask & (current_data); *set_address = clock_mask;
#define latch() *set_address = latch_pin; *clr_address = latch_pin;

void initialize_dacs()
{
    for (int i = 16; i <= 25; i++)  // Pin stats for latch clock  and data lines
        gpio_function_select(i, GPFSEL_Output);
    
    s_load_sample_rates_from_file();
    s_load_output_configs_from_file();
    s_set_defult_resolution();
    s_set_defult_buffering_states();
}

int _dac_output_start_internal(void* buffer_start, size_t n, int flags)
{
    const uint32_t latch_pin = 1 << 16;
    const uint32_t clock_mask = 1 << 17;
    const uint32_t data_mask = s_create_data_mask_from_flags(flags);
    const bool should_loop = !(flags & DAC_OUTPUT_FLAGS_DONT_LOOP);
    const int frames_per_value = s_current_resolution / 2;
    const int shit_amount = 1 << ((flags  + 1) & 3);
    const int frames_per_u64 = 64 / shit_amount;
    const int values_per_u64 = frames_per_u64 / frames_per_value;

    volatile uint32_t* set_address = get_mmio_pointer(GPSET0);
    volatile uint32_t* clr_address = get_mmio_pointer(GPCLR0);

    uint64_t* ptr = (uint64_t*)buffer_start;
    uint64_t current_data = *ptr++;
    size_t count = n;

    int error_bits = s_check_if_flag_channels_support_current_state(flags);

    if (error_bits)
        return error_bits;

    while (1)
    {
        __builtin_prefetch(ptr);

        for (int i = 0; i < values_per_u64 && count; i++)
        {
            for (int j = 0; j < frames_per_value; j++)
            {
                shift_register_out();
                rotate_right(current_data, current_data, shit_amount);
            }
            latch();
            count--;
        }

        if (count)
        {
            current_data = *ptr++;
            continue;
        }

        if (should_loop)
        {
            ptr = (uint64_t*)buffer_start;
            current_data = *ptr++;
            count = n;
        }
        else
        {
            break;
        }
    }

    return 0;
}

int dac_resolution(int resolution)
{
    if (resolution == -1)           
        return s_current_resolution;

    uint8_t controll_register_value = *hardware_controll_register_relay_byte;
    controll_register_value &= ~0x30;   // Remove the resolution controll bits
    int new_sample_rate = 0;


    switch (resolution)
    {
    case 2:
        controll_register_value |= 0x30;
        new_sample_rate = s_sample_rate_2_bit;
        break;
    case 8: // commented out since or 0 does nothing
        // controll_register_value |= 0x00;
        new_sample_rate = s_sample_rate_8_bit;
        break;
    case 16:
        controll_register_value |= 0x20;
        new_sample_rate = s_sample_rate_16_bit;
        break;
    default:
        return s_current_resolution;
    }

    if (new_sample_rate == 0)           // Dont do anything is the resolution is dissabled globaly
        return s_current_resolution;

    *hardware_controll_register_relay_byte = controll_register_value;
    hardware_controll_register_write();

    s_current_resolution = resolution;

    return resolution;
}

int dac_channel_buffering(int channel, int state)
{
    if (channel < 1 || channel > 4)
        return -1;

    channel--;

    if (state == -1)
        return s_dacs[channel].is_buffered ? 1 : 0;

    uint8_t bypass_bit = 1 << (3 - channel);

    if (state == 0)
    {
        if (s_dacs[channel].supports_state_unbuffered == false)
            return 1;   // Dont do anything if the state is unsuported

        *hardware_controll_register_relay_byte |= bypass_bit;
        s_dacs[channel].is_buffered = false;
    }
    else
    {
        if (s_dacs[channel].supports_state_buffered == false)
            return 0;    // Dont do anything if the state is unsuported

        *hardware_controll_register_relay_byte &= ~bypass_bit;
        s_dacs[channel].is_buffered = true;
    }

    hardware_controll_register_write();

    return s_dacs[channel].is_buffered ? 1 : 0;
}

int dac_get_sample_rate(int resolution)
{
    if (resolution == -1)
        resolution = s_current_resolution;

    switch (resolution)
    {
    case 2:
        return s_sample_rate_2_bit;
    case 8:
        return s_sample_rate_8_bit;
    case 16:
        return s_sample_rate_16_bit;
    default:
        return 0;
    }
}

int dac_channel_supports_config(int channel, int resolution, int buffered)
{
    if (channel < 1 || channel > 4)
        return 1 << 2;

    channel--;

    if (resolution == -1)
        resolution = s_current_resolution;

    if (buffered == -1)
        buffered = s_dacs[channel].is_buffered ? 1 : 0;

    int value_to_check = 0;
    int return_value = 0;
    
    if (buffered == 0)
        value_to_check = s_dacs[channel].supports_state_unbuffered;
    else
        value_to_check = s_dacs[channel].supports_state_buffered;

    if (value_to_check == 0)
        return_value |= 1 << 0;

    switch (resolution)
    {
    case 2:
        value_to_check = s_dacs[channel].supports_resolution_2_bit;
        break;
    case 8:
        value_to_check = s_dacs[channel].supports_resolution_8_bit;
        break;
    case 16:
        value_to_check = s_dacs[channel].supports_resolution_16_bit;
        break;
    default:
        break;
    }

    if (value_to_check == 0)
        return_value |= 1 << 1;

    return return_value; 
}

uint32_t s_create_data_mask_from_flags(int flags)
{
    uint32_t mask = 0;

    if (flags & DAC_OUTPUT_FLAGS_CH1_ENABLED)
        mask |= 1 << 18 | 1 << 19;

    if (flags & DAC_OUTPUT_FLAGS_CH2_ENABLED)
        mask |= 1 << 20 | 1 << 21;

    if (flags & DAC_OUTPUT_FLAGS_CH3_ENABLED)
        mask |= 1 << 22 | 1 << 23;

    if (flags & DAC_OUTPUT_FLAGS_CH4_ENABLED)
        mask |= 1 << 24 | 1 << 25;

    return mask;
}

static void s_load_sample_rates_from_file()
{
    config_file header;

    if (load_config_file(&header, "config/outputs.cfg") == false)
        return;

    s_sample_rate_16_bit = get_u64_from_config_file_entry_with_defult_by_name(&header, "SAMPLE_RATE_16_BIT", 0);
    s_sample_rate_8_bit = get_u64_from_config_file_entry_with_defult_by_name(&header, "SAMPLE_RATE_8_BIT", 0);
    s_sample_rate_2_bit = get_u64_from_config_file_entry_with_defult_by_name(&header, "SAMPLE_RATE_2_BIT", 0);

    free_loaded_config_file(&header);
}

static void s_load_output_configs_from_file()
{
    memclr(s_dacs, sizeof(s_dacs));

    s_load_output_config_from_file(s_dacs, "config/channel1.cfg");
    s_load_output_config_from_file(s_dacs + 1, "config/channel2.cfg");
    s_load_output_config_from_file(s_dacs + 2, "config/channel3.cfg");
    s_load_output_config_from_file(s_dacs + 3, "config/channel4.cfg");
}

static void s_load_output_config_from_file(dac_info* output, const char* path)
{
    config_file header;

    if (load_config_file(&header, path) == false)
        return;

    output->supports_resolution_16_bit = get_u64_from_config_file_entry_with_defult_by_name(&header, "SUPPORTS_RESOLUTION_16_BIT", 0);
    output->supports_resolution_8_bit = get_u64_from_config_file_entry_with_defult_by_name(&header, "SUPPORTS_RESOLUTION_8_BIT", 0);
    output->supports_resolution_2_bit = get_u64_from_config_file_entry_with_defult_by_name(&header, "SUPPORTS_RESOLUTION_2_BIT", 0);
    output->supports_state_unbuffered = get_u64_from_config_file_entry_with_defult_by_name(&header, "SUPPORTS_STATE_UNBUFFERED", 0);
    output->supports_state_buffered = get_u64_from_config_file_entry_with_defult_by_name(&header, "SUPPORTS_STATE_BUFFERED", 0);

    free_loaded_config_file(&header);
}

static int s_check_if_flag_channels_support_current_state(int flags)
{   
    if (flags & DAC_OUTPUT_FLAGS_CH1_ENABLED && dac_channel_supports_config(1, -1, -1))
        return 1;

    if (flags & DAC_OUTPUT_FLAGS_CH2_ENABLED && dac_channel_supports_config(2, -1, -1))
        return 2;

    if (flags & DAC_OUTPUT_FLAGS_CH3_ENABLED && dac_channel_supports_config(3, -1, -1))
        return 3;

    if (flags & DAC_OUTPUT_FLAGS_CH4_ENABLED && dac_channel_supports_config(4, -1, -1))
        return 4;

    return 0;
}

static void s_set_defult_resolution()
{
    // 8 bit -> 16 -> 2

    if (s_sample_rate_8_bit)
        dac_resolution(8);
    else if (s_sample_rate_16_bit)
        dac_resolution(16);
    else if (s_sample_rate_2_bit)
        dac_resolution(2);
    else
        printf("Error: No suported resolutions detected.\n");
}

void s_set_defult_buffering_states()
{
    for (int i = 0; i < 4; i++)
    {
        if (s_dacs[i].supports_state_buffered)
            dac_channel_buffering(i + 1, 1);
        else if (s_dacs[i].supports_state_unbuffered)
            dac_channel_buffering(i + 1, 0);
        else
            printf("Warning: No vaild buffer state for channel %d detected.\n", i + 1);
    }
}
