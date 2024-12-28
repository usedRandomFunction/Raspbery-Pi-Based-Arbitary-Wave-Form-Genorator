#include "common/program_managment.h"
#include "abstracted_outputs/raw.h"
#include "common/basic_io.h"
#include "common/keypad.h"
#include "common/memory.h"
#include "common/string.h"
#include "common/output.h"
#include "common/math.h"
#include "numeric_input.h"

// The main loop that handles key presses
int application_main_loop();

// Handles keydown events
// @param keys The keys that have been pressed
// @note More then only key may be sent in this event
void on_key_down(keypad_state key);

// Attempts to toggle the buffering state of
// the given channel
void toggle_channel_buffering(int channel);

// Prints details about what the user choose.
void print_selection_choices();

// Sets selection_choice using keypreses
// @param key_down The keys previded by on_key_down
void set_selection_choice(keypad_state key_down);

// Prints the wave types to choose from
void print_wave_type_seleciton_choices();

// Sets wave type using keypad input
// @param key_down The keys previded by on_key_down
void handle_wave_type_selection(keypad_state key_down);

// Allocates and writes the buffer, then outputs it
void start_output();

// Handler for PRG_EXIT, sets selection_choice to 0
void abort_buffer_write();

// Function for square wave
// @param x Angle in radians
// @return Value [0, 1]
double square_wave(double x);

// Function for triangle wave
// @param x Angle in radians
// @return Value [0, 1]
double triangle_wave(double x);

// Function for saw wave
// @param x Angle in radians
// @return Value [0, 1]
double saw_wave(double x);

// Function for inverse saw wave
// @param x Angle in radians
// @return Value [0, 1]
double inverse_saw_wave(double x);

typedef double (*WAVE_FUNCTION)(double);

const WAVE_FUNCTION wave_functions[] = {sin, square_wave, triangle_wave, saw_wave, inverse_saw_wave};
const char* wave_type_strings[] = {"Sine", "Square", "Triangle", "Saw", "Inverse saw"};


double_input_data selections[5];

size_t output_buffer_size = 0;
void* output_buffer = (void*)0xC0000000;

WAVE_FUNCTION selected_wave;
int selection_choice = 0;

int main()
{
    if (dac_resolution(8) != 8)
    {
        printf("Failed to set DAC resolution\n");
        return -1;
    }

    if (dac_channel_supports_config(1, 8, -1))
    {
        printf("CH1 does not support current resolution.\n");
        return -1;
    }

    print_selection_choices();

    memclr(selections, sizeof(selections));

    selections[0].use_prefix = true;
    selections[0].min_string = "0.01";
    selections[0].name = "Frequency";
    selections[0].unit = "Hz";
    selections[0].max = dac_get_sample_rate(-1) / 2;
    selections[0].min = 0.01;
    selections[0].value = 1000;
    strcpy_s( "1000", 32, selections[0].value_string);

    selections[1].use_prefix = true;
    selections[1].min_string = "-180";
    selections[1].name = "Phase offset";
    selections[1].unit = "Deg";
    selections[1].max = 180;
    selections[1].min = -180;
    selections[1].value = 0;
    strcpy_s( "0", 32, selections[1].value_string);

    selections[2].use_prefix = true;
    selections[2].min_string = "0";
    selections[2].name = "Amplitude";
    selections[2].unit = "V";
    selections[2].max = 5;
    selections[2].min = 0;
    selections[2].value = 1;
    strcpy_s("1", 32, selections[2].value_string);

    selections[3].use_prefix = true;
    selections[3].min_string = "-10";
    selections[3].name = "DC offset";
    selections[3].unit = "V";
    selections[3].max = 10;
    selections[3].min = -10;
    selections[3].value = 2.5;
    strcpy_s("2.5", 32, selections[3].value_string);

    selections[4].use_prefix = true;
    selections[4].min_string = "0";
    selections[4].name = "Duty cycle";
    selections[4].unit = "%";
    selections[4].max = 100;
    selections[4].min = 0;
    selections[4].value = 50;
    strcpy_s( "50", 32, selections[4].value_string);

    selected_wave = sin;

    return application_main_loop();
}

int application_main_loop()
{
    keypad_state previous_keypad_state = 0;

    while (1)
    {
        keypad_state current_keypad_state = get_keypad_state();

        // This mess finds out what keys have just started to be pressed
        const keypad_state key_down = (current_keypad_state ^ previous_keypad_state) & current_keypad_state;
        previous_keypad_state = current_keypad_state;

        if (key_down != 0)
            on_key_down(key_down);
    }

    return 0;
}

void on_key_down(keypad_state key)
{
    if (key & KEYPAD_STATE_BUTTON_CH1_BUFFER)
        toggle_channel_buffering(1);

    if (selection_choice == 0)
        set_selection_choice(key); 
    else if (selection_choice == 1)
        handle_wave_type_selection(key);
    else
    {
        if (numeric_input_handle_input(key))
        {
            selection_choice = 0;

            print_selection_choices();
        }
    }
}

void toggle_channel_buffering(int channel)
{
    const int current_state = dac_channel_buffering(channel, -1);
    const int target_state = 1 - current_state;

    const char* state_string = target_state ? "Enabled" : "Disabled";

    if (dac_channel_buffering(channel, target_state) == target_state)
    {
        printf("%s buffering on channel %d\n", state_string, channel);
        return;         // Success
    }
    
    printf("Failed to %s buffering on channel %d\n", state_string, channel);
}

void print_selection_choices()
{
    const char* option_6 = selected_wave ==  square_wave ? "6: Duty cycle\n" : "";

    printf(
        "Simple channel 1 wave, selection choices:\n"
        "1: Wave type\n"
        "2: Frequency\n"
        "3: Phase offset\n"
        "4: Amplitude\n"
        "5: DC offset\n%s"
        "CH1: Start output\n\n", option_6);
}

void set_selection_choice(keypad_state key_down)
{
    const int digit = get_digiet_from_keypad_state(key_down);
    const int max_selection_option = selected_wave ==  square_wave ? 6 : 5;

    if (digit >= 1 && digit <= max_selection_option)
        selection_choice = digit;

    if (selection_choice != 0 && selection_choice != 1)
    {
        putchar('\n');
        prepare_numeric_input_for_double(&selections[digit - 2]);
    }

    if (selection_choice == 1)
        print_wave_type_seleciton_choices();

    if (key_down & KEYPAD_STATE_BUTTON_CH1)
    {
        start_output();
        print_selection_choices();
    }
}

void print_wave_type_seleciton_choices()
{
    printf(
        "Avalible wave types:\n"
        "1: Sine\n"
        "2: Square\n"
        "3: Triangle\n"
        "4: Saw\n"
        "5: Inverse saw\n\n");
}

void handle_wave_type_selection(keypad_state key_down)
{
    if (key_down & KEYPAD_STATE_BUTTON_DEL || key_down & KEYPAD_STATE_BUTTON_CLR || key_down & KEYPAD_STATE_BUTTON_ENT)
    {
        printf("Aborted\n");
        selection_choice = 0;
        print_selection_choices();
        return;
    }

    const int digit = get_digiet_from_keypad_state(key_down) - 1;

    if (digit < 0 || digit > 4)
        return;

    selected_wave = wave_functions[digit];

    printf("%s wave selected\n", wave_type_strings[digit]);
    print_selection_choices();
    selection_choice = 0;
}

void start_output()
{
    const uint32_t samples_per_second = dac_get_sample_rate(-1);
    selection_choice = -1;

    const int required_samples = samples_per_second / selections[0].value;  // Frequency
    const double conversion_factor = M_PI_M_2 / required_samples;

    // There are 8 samples to a 8 byte buffer entry, so round up
    const size_t required_buffer_size = required_samples + ((required_samples % 8) ? (8 - required_samples % 8) : 0); 
    printf("%d Samples\n%d Bytes required\n", required_samples, required_buffer_size);


    if (output_buffer_size < required_buffer_size)
    {
        if (vmemmap(output_buffer, required_buffer_size, VMEMMAP_WRITABILITY) < required_buffer_size)
        {
            printf("Failed to allocate buffer\n");
            return;
        }

        output_buffer_size = required_buffer_size;
    }

    const double phase_offset = selections[1].value / 180 * M_PI;
    const double dc_offset = selections[3].value;
    const double amplitude = selections[2].value;


    printf("Writing buffer, press PRG_EXIT to abort\n");
    capture_prg_exit(abort_buffer_write);

    for (int i = 0; i < required_samples && selection_choice; i++)
    {
        float voltage = dc_offset + amplitude * selected_wave(conversion_factor * i + phase_offset);

        int value = voltage_to_dac_value(voltage, 8);

        write_dac_buffer_entry(output_buffer, value, i, 0, 8, 2, 18);
    }

    capture_prg_exit(NULL);

    if (selection_choice == 0)
    {
        printf("Aborted!\n");
        return;
    }

    selection_choice = 0;   // We dont need to keep track of this anymore

    capture_prg_exit(dac_output_end);
    keypad_polling(0);

    printf("Output live, keypad dissabled\nPress PRG_EXIT to stop");
    dac_output_start(output_buffer, required_samples, DAC_OUTPUT_FLAGS_CH1_ENABLED | DAC_OUTPUT_FLAGS_FRAME_SIZE_2_BITS);

    capture_prg_exit(NULL);
    keypad_polling(-2);
}

void abort_buffer_write()
{
    selection_choice = 0;
}

double square_wave(double x)
{
    x = fmod(x, M_PI_M_2);

    x /= M_PI_M_2 / 100;

    if (x < selections[4].value)
        return 1;

    return 0;
}

double triangle_wave(double x)
{
    x -= M_PI;

    x = fmod(x, M_PI_M_2);

    x /= M_PI;

    return fabs(1 - x);
}

double saw_wave(double x)
{
    x = fmod(x, M_PI_M_2);

    return x / M_PI_M_2;
}

double inverse_saw_wave(double x)
{
    return 1 - saw_wave(x);
}