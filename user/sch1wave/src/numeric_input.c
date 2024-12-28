#include "numeric_input.h"

#include "common/basic_io.h"
#include "common/memory.h"
#include "common/string.h"
#include "common/math.h"

static double_input_data* s_target_location = NULL;
static double_input_data s_working_data;
static double s_decimal_multiplyer;
static bool s_was_last_input_base;


const char* scientific_notation[] = {"*10^-6 ", "*10^-3 ", "       ", "*10^3  ", "*10^6  "};
const char* prefixes[] = {"u", "m", " ", "k", "M"};

// Handles return logic for numeric_input_handle_input, for when enter is pressed
static int s_get_values_ready_for_return();

// Logic for handling the delete button for numeric_input_handle_input
static void s_handle_delete();

// Logic for handling the plus minus button for numeric_input_handle_input
static void s_handle_plusminus();

// Logic for handling clear for numeric_input_handle_input
static void s_handle_clear();

// Prints the values from s_working_data
static void s_print_input_object();

// Logic for handling base selection (Keys A B C)
// @param key_down New keys as provied by on_key_down
static void s_handle_base_selection(keypad_state key_down);

// Logic for handling digits Keys 0 though 9
// @param key_down New keys as provied by on_key_down
static void s_handle_digits(keypad_state key_down);

// Logic for adding the decimal point
static void s_handle_decimal_point();

// Attempts to insert a charector into the value string of s_working_data
// @param ch Charector to insert
// @return 0 If success 1 if failed
static int s_try_to_insert_char_to_value_string(char ch);

int get_digiet_from_keypad_state(keypad_state state)
{
    if (state & KEYPAD_STATE_BUTTON_9)
        return 9;
    else if (state & KEYPAD_STATE_BUTTON_8)
        return 8;
    else if (state & KEYPAD_STATE_BUTTON_7)
        return 7;
    else if (state & KEYPAD_STATE_BUTTON_6)
        return 6;
    else if (state & KEYPAD_STATE_BUTTON_5)
        return 5;
    else if (state & KEYPAD_STATE_BUTTON_4)
        return 4;
    else if (state & KEYPAD_STATE_BUTTON_3)
        return 3;
    else if (state & KEYPAD_STATE_BUTTON_2)
        return 2;
    else if (state & KEYPAD_STATE_BUTTON_1)
        return 1;
    else if (state & KEYPAD_STATE_BUTTON_0)
        return 0;

    return -1;
}

void prepare_numeric_input_for_double(double_input_data* value)
{
    memcpy(&s_working_data, value, sizeof(double_input_data));
    s_was_last_input_base = false;
    s_target_location = value;
    s_decimal_multiplyer = 1;

    s_print_input_object();
}

int numeric_input_handle_input(keypad_state key_down)
{
    if (s_target_location == NULL)  // Just do nothing if we are called, but are not active
        return 0;

    s_handle_base_selection(key_down);
    s_handle_digits(key_down);

    if (key_down & KEYPAD_STATE_BUTTON_DOT)
    {
        s_handle_decimal_point();
        s_was_last_input_base = false;
    }


    if (key_down & KEYPAD_STATE_BUTTON_PLUSMINUS)
    {
        s_handle_plusminus();
        s_was_last_input_base = false;
    }

    if (key_down & KEYPAD_STATE_BUTTON_CLR)
    {
        s_was_last_input_base = false;
        s_handle_clear();
    }

    if (key_down & KEYPAD_STATE_BUTTON_DEL)
    {
        s_was_last_input_base = false;
        s_handle_delete();
    }

    
    if (key_down & KEYPAD_STATE_BUTTON_ENT)
    {
        int return_value = s_get_values_ready_for_return();
        putchar('\n');

        s_target_location = NULL;

        return return_value;
    }

    return 0;
}

int s_get_values_ready_for_return()
{
    if (s_working_data.value_string[0] == '\0')
        return -1;

    s_working_data.value *= pow(10, 3 * s_working_data.base);

    if (s_working_data.value < s_working_data.min)
    {   
        s_working_data.base = s_working_data.min_base;
        s_working_data.value = s_working_data.min;
        strcpy_s(s_working_data.min_string, 32, s_working_data.value_string);
    }

    memcpy(s_target_location, &s_working_data, sizeof(double_input_data));

    return 1;
}

void s_handle_delete()
{
    const size_t index = strlen(s_working_data.value_string);

    if (index == 0)     // Dont delete anything if the size is 0
        return;

    const char last_char = s_working_data.value_string[index - 1];
    s_working_data.value_string[index - 1] = '\0';

    if (last_char == '.')
    {
        s_decimal_multiplyer = 1;   // This is used to repersent that the decimal point was removed
    }
    else
    {
        int last_digit = (int)last_char - 0x30;

        s_working_data.value -= ((double)last_digit) * s_decimal_multiplyer;

        if (s_decimal_multiplyer == 1)
            s_working_data.value /= 10;
    }


    s_print_input_object();
    putchar(' ');               // Add one space to remove the last chareactor from the last line
}

void s_handle_plusminus()
{
    bool needs_extra_space_after_printing = false;

    if (s_was_last_input_base)
    {
        const double new_base = -s_working_data.base;
        const double true_value = s_working_data.value * pow(10, new_base * 3);
        
        if (true_value < s_working_data.min || true_value > s_working_data.max)
            return;         // If the new value will be out of bounds, dont do anything

        s_working_data.base = new_base;
    }
    else
    {
        const double new_value = -s_working_data.value;
        const double true_value = new_value * pow(10, s_working_data.base * 3);

        if (true_value < s_working_data.min || true_value > s_working_data.max)
            return;         // If the new value will be out of bounds, dont do anything

        s_working_data.value = new_value;

        needs_extra_space_after_printing = new_value > 0; // If the value WAS negitive then we need a new space

    }

    s_print_input_object();

    if (needs_extra_space_after_printing)
        putchar(' '); 
}

void s_handle_clear()
{
    size_t value_string_size = strlen(s_working_data.value_string);

    s_working_data.value_string[0] = '\0';
    s_working_data.value = 0;
    s_working_data.base = 0;

    s_print_input_object();

    while (value_string_size--)
        putchar(' ');               // Print spaces to over write the old text on the screen
}

void s_print_input_object()
{
    const char** base_strings = ((s_working_data.use_prefix) ? prefixes : scientific_notation);
    const char* base_string = base_strings[s_working_data.base + 2];
    const char* number_prefix = (s_working_data.value >= 0) ? "" : "-";

    // {NAME}: {' '/'-'}{VALUE_STRING} {BASE} {UNIT}

    printf("\r%s: %s%s %s%s", s_working_data.name, number_prefix, s_working_data.value_string, base_string, s_working_data.unit);
}

void s_handle_base_selection(keypad_state key_down)
{
    int base_selection = -1;

    if (key_down & KEYPAD_STATE_BUTTON_A)
        base_selection = 2;
    else if (key_down & KEYPAD_STATE_BUTTON_B)
        base_selection = 1;
    else if (key_down & KEYPAD_STATE_BUTTON_C)
        base_selection = 0;

    if (base_selection < 0)
        return;                 // Return if none of the relivent buttons were pressed

    s_was_last_input_base = true;

    if (s_working_data.base < 0)
        base_selection *= -1;   // If the current base is negitive, the new one will be as well


    const double true_value = s_working_data.value * pow(10, base_selection * 3);

    if (true_value < s_working_data.min || true_value > s_working_data.max)
            return;         // If the new value will be out of bounds, dont do anything

    s_working_data.base = base_selection;

    s_print_input_object();
}

void s_handle_digits(keypad_state key_down)
{
    const int digit = get_digiet_from_keypad_state(key_down);

    if (digit == -1)
        return;         // Return if none of the relivent buttons were pressed

    s_was_last_input_base = false;

    double new_value = s_working_data.value;

    if (s_decimal_multiplyer == 1)
        new_value *= 10;

    if (new_value >= 0)         // Handle negitive values appropriately 
        new_value += digit * s_decimal_multiplyer;
    else
        new_value -= digit * s_decimal_multiplyer;

    const double true_value = new_value * pow(10, s_working_data.base * 3);

    if (true_value < s_working_data.min || true_value > s_working_data.max)
        return;         // If the new value will be out of bounds, dont do anything

    if (s_working_data.value == 0 && s_working_data.value_string[0] == '0' && s_working_data.value_string[1] == '\0')
        s_working_data.value_string[0] = '\0';

    if (s_try_to_insert_char_to_value_string((char)(digit + 0x30)))
        return;         // If there is no more space to insert chars into, dont do anything

    if (s_decimal_multiplyer != 1)
        s_decimal_multiplyer /= 10;
    s_working_data.value = new_value;

    s_print_input_object();
}

void s_handle_decimal_point()
{
    if (s_decimal_multiplyer != 1)
        return;         // Dont do anything if there is allready a decimal point

    if (s_try_to_insert_char_to_value_string('.'))
        return;         // If there is no more space to insert chars into, dont do anything

    s_decimal_multiplyer = 0.1;

    s_print_input_object();
}

int s_try_to_insert_char_to_value_string(char ch)
{
    const size_t index = strlen(s_working_data.value_string);

    if (index >= 31)
        return 1;       // Not enough space to insert into

    s_working_data.value_string[index] = ch;
    s_working_data.value_string[index + 1] = '\0';

    return 0;
}