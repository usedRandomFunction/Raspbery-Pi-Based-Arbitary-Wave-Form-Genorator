#ifndef NUMERIC_INPUT_H
#define NUMERIC_INPUT_H

#include "common/keypad.h"

#include <stdbool.h>

struct double_input_data
{
    double value;
    char* name;
    char* unit;

    char* min_string;   // If the enter value is < min, value is set to min and min_string is copyed into value_string
    int min_base;       // Base used by min_string
    double max;
    double min;

    bool use_prefix;
    int base;

    char value_string[32];
};

typedef struct double_input_data double_input_data;


// Gets a signal digiet from the give keypad state.
// If more then one of the [0, 9] buttons are pressed
// the highest value will be reutrned
// @param state The keypad state to get the digiet from
// @return [0, 9] for the button, or -1 if no number keys are pressed
int get_digiet_from_keypad_state(keypad_state state);

// Sets the numeric input to active and to use `value` as the output
// @param value The double_input_data struct to save the value to
void prepare_numeric_input_for_double(double_input_data* value);

// Handles keypad input, prints to sreen the value being edited
// @param key_down New keys as provied by on_key_down
// @return 0 if input still need to run next frame, < 0 if input ended, but was aborted / value did not change, > 0 if success
int numeric_input_handle_input(keypad_state key_down);

#endif