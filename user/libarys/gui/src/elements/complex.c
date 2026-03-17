#include "gui/elements/complex.h"
#include "common/basic_io.h"
#include "common/memory.h"
#include "common/alloc.h"
#include "common/string.h"
#include <stdbool.h>

static const char* s_scientific_notation[] = {"*10^-6 ", "*10^-3 ", "*10^0", "*10^+3 ", "*10^+6 "};
static const char* s_matrix_prefixes[] = {"u", "m", " ", "k", "M"};
static const float s_magnitudes[] = {1e-6, 1e-3, 1, 1e3, 1e6}; 


int initialize_integer_input_element(gui_element* element, int padding, int size_x_char)
{
    initialize_standered_element_values(element);
    element->draw = gui_complex_element_integer_input_draw_function;


    gui_complex_element_data* complex_data = malloc(sizeof(gui_complex_element_data));

    if (complex_data == NULL)
    {
        printf("[Error] Failed to initialize intieger input element, malloc failed\n");
        return 1;
    }

    memclr(complex_data, sizeof(gui_complex_element_data));
    complex_data->on_captured_key_down = gui_complex_element_integer_input_on_captured_key_down;
    complex_data->on_cursor_toggle = gui_complex_element_integer_input_on_cursor_toggle;
    complex_data->on_capture_end = gui_complex_element_integer_input_on_capture_end;

    gui_complex_element_integer_input_data* data = malloc(sizeof(gui_complex_element_integer_input_data));

    if (data == NULL)
    {
        printf("[Error] Failed to initialize intieger input element, malloc failed\n");
        free(complex_data);
        return 2;
    }

    memclr(data, sizeof(gui_complex_element_integer_input_data));
    gui_standard_element_text_box_data* text_data = &data->text_data;
    element->data = text_data;
    size_textbox_element_for_n_characters(element, size_x_char, padding);
    
    


    element->flags |= GUI_ELEMENT_FLAGS_CAN_CAPTURE_INPUT;
    element->data = complex_data;
    complex_data->data = data;

    return 0;
}


gui_element* create_integer_input_element(int64_t default_value, int64_t minimum, int64_t maximum, int padding, int size_x_char, dynamic_array* buffer)
{
    gui_element* element = create_element(buffer);

    if (!element)
        return NULL;

    
    // Handle initialization errors
    if (initialize_integer_input_element(element, padding, size_x_char))
    {
        free(element);

        return NULL;
    }

    gui_complex_element_data* complex_data = element->data;
    gui_complex_element_integer_input_data* data = complex_data->data;

    data->default_v = default_value;
    data->value = default_value;
    data->minimum = minimum;
    data->maximum = maximum;
    data->recently_cleared = false;

    return element;
}

// TODO dont let more then the maximum number of chars write
void gui_complex_element_integer_input_on_captured_key_down(gui_element* element, gui_event* event, gui_application* app)
{
    if (!element || !event || !app || event->event_type != GUI_EVENT_TYPE_KEY_DOWN || !event->event_data)
        return;         // Sanity Check, i dont think this will ever run.

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_integer_input_data* data = (gui_complex_element_integer_input_data*)base_data->data;
    keypad_state key = *(keypad_state*)event->event_data;
 
    data->recently_cleared = false; 
    // Handle key inputs

    switch (key) 
    {
    case KEYPAD_STATE_BUTTON_ENT:
        gui_application_set_input_capture(app, NULL);
        return;
    case KEYPAD_STATE_BUTTON_PLUSMINUS:
        data->value *= -1;
        break;
    case KEYPAD_STATE_BUTTON_DEL:
        data->value /= 10;
        
        if (data->value == 0)
            data->recently_cleared = true;
        break;
    case KEYPAD_STATE_BUTTON_CLR:
        data->recently_cleared = true;
        data->value = 0;
        break;
    case KEYPAD_STATE_BUTTON_0:
    case KEYPAD_STATE_BUTTON_1:
    case KEYPAD_STATE_BUTTON_2:
    case KEYPAD_STATE_BUTTON_3:
    case KEYPAD_STATE_BUTTON_4:
    case KEYPAD_STATE_BUTTON_5:
    case KEYPAD_STATE_BUTTON_6:
    case KEYPAD_STATE_BUTTON_7:
    case KEYPAD_STATE_BUTTON_8:
    case KEYPAD_STATE_BUTTON_9:
        int64_t digit = get_digiet_from_keypad_state(key);
        data->value *= 10;

        if (data->value >= 0)
            data->value += digit;
        else
            data->value -= digit;
        break;
    default: // Did not change anything, we will ignore it completly
        return;
    } 

    
    draw_element(element, app->target_buffer);
}


void gui_complex_element_integer_input_on_cursor_toggle(gui_element* element, gui_event* event, gui_application* app)
{
    if (!element || !app || !element->data)
        return;

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_integer_input_data* data = (gui_complex_element_integer_input_data*)base_data->data;

    data->text_data.cursor_visible = !data->text_data.cursor_visible;

    draw_element(element, app->target_buffer);
}


void gui_complex_element_integer_input_on_capture_end(gui_element* element, gui_event* event, gui_application* app)
{
    if (!element || !app || !element->data)
        return;

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_integer_input_data* data = (gui_complex_element_integer_input_data*)base_data->data;

    if (data->value < data->minimum)
        data->value = data->minimum;
    else if (data->value > data->maximum)
        data->value = data->maximum;
    else if (data->recently_cleared)
        data->value = data->default_v;

    data->text_data.cursor_visible = false;
    data->recently_cleared = false;

    draw_element(element, app->target_buffer);

    event = malloc(sizeof(gui_event));

    if (!event)
    {
        printf("[Error] Failed to create event for integer input changed: malloc failed!");
        return;
    }

    memclr(event, sizeof(gui_event));
    event->event_type = GUI_EVENT_TYPE_INPUT_FEILD_INT_CHANGED;
    event->event_data = element;

    gui_event_queue_push(&app->event_queue, event);
}


void gui_complex_element_integer_input_draw_function(gui_element* element, gui_vec2 offset, int target_buffer)
{
    if (!element  || !element->data)
        return;

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_integer_input_data* data = (gui_complex_element_integer_input_data*)base_data->data;

    char text_buffer[255];
    memclr(text_buffer, 255); // This feels a bit slow tbh
    
    if (!data->recently_cleared) // So it its blank if you press CLR
        sprintf_s(text_buffer, 255, "%d", data->value);
    
    data->text_data.str = text_buffer;

    // Now (temporarlly) assign the text data as the element's data
    element->data = &data->text_data;
    
    gui_standard_element_textbox_draw_function(element, offset, target_buffer);

    element->data = base_data;
    data->text_data.str = NULL;
}



int initialize_float_input_element(gui_element* element, int padding, int size_x_char)
{
    initialize_standered_element_values(element);
    element->draw = gui_complex_element_float_input_draw_function;


    gui_complex_element_data* complex_data = malloc(sizeof(gui_complex_element_data));

    if (complex_data == NULL)
    {
        printf("[Error] Failed to initialize float input element, malloc failed\n");
        return 1;
    }

    memclr(complex_data, sizeof(gui_complex_element_data));
    complex_data->on_captured_key_down = gui_complex_element_float_input_on_captured_key_down;
    complex_data->on_cursor_toggle = gui_complex_element_float_input_on_cursor_toggle;
    complex_data->on_capture_end = gui_complex_element_float_input_on_capture_end;

    gui_complex_element_float_input_data* data = malloc(sizeof(gui_complex_element_float_input_data));

    if (data == NULL)
    {
        printf("[Error] Failed to initialize float input element, malloc failed\n");
        free(complex_data);
        return 2;
    }

    memclr(data, sizeof(gui_complex_element_integer_input_data));
    gui_standard_element_text_box_data* text_data = &data->text_data;
    element->data = text_data;
    size_textbox_element_for_n_characters(element, size_x_char, padding);
    data->max_input_len = size_x_char > 31 ? 31 : size_x_char;  // Clamp it so there are no buffer overuns.

    element->flags |= GUI_ELEMENT_FLAGS_CAN_CAPTURE_INPUT;
    element->data = complex_data;
    complex_data->data = data;

    return 0;
}


gui_element* create_float_input_element(bool show_magnitude_as_sci, bool allow_magnitude_change,
                                        float default_coefficent, uint8_t default_magnitude, const char* default_string,
                                        float decimal_multiplyer_at_defult,
                                        float maximum_coefficent, uint8_t maximum_magnitide, const char* maximum_string, 
                                        float decimal_multiplyer_at_maximum,
                                        float minimum_coefficent, uint8_t minimum_magnitide, const char* minimum_string,
                                        float decimal_multiplyer_at_minimum,
                                        const char* unit, int padding, int size_x_char, dynamic_array* buffer)
{
    gui_element* element = create_element(buffer);

    if (!element)
        return NULL;

    
    // Handle initialization errors
    if (initialize_float_input_element(element, padding, size_x_char))
    {
        free(element);

        return NULL;
    }

    gui_complex_element_data* complex_data = element->data;
    gui_complex_element_float_input_data* data = complex_data->data;

    data->show_magnitude_as_sci = show_magnitude_as_sci;
    data->allow_magnitude_change = allow_magnitude_change;

    data->magntiude_at_default = default_magnitude; 
    data->magntiude_at_maximum = maximum_magnitide;
    data->magntiude_at_minimum = minimum_magnitide;

    data->default_coefficent = default_coefficent;
    data->maximum_coefficent = maximum_coefficent;
    data->minimum_coefficent = minimum_coefficent;

    data->default_str = default_string;
    data->maximum_str = maximum_string;
    data->minimum_str = minimum_string;

    data->decimal_multiplyer_at_maximum = decimal_multiplyer_at_maximum;
    data->decimal_multiplyer_at_minimum = decimal_multiplyer_at_minimum;
    data->decimal_multiplyer_at_defult = decimal_multiplyer_at_defult;
    data->decimal_multiplyer = data->decimal_multiplyer_at_defult;

    data->current_coefficent = data->default_coefficent;
    data->current_magnitude = data->magntiude_at_default;
    strcpy_s(data->default_str, 32, data->current_str);
    data->current_input_len = strlen(data->current_str);

    data->true_maximum = data->maximum_coefficent * s_magnitudes[data->magntiude_at_maximum + 2];
    data->true_minimum = data->minimum_coefficent * s_magnitudes[data->magntiude_at_minimum + 2];
    data->true_default = data->default_coefficent * s_magnitudes[data->magntiude_at_default + 2];
    data->output = data->true_default;
    data->unit = unit;

    return element;
}

void gui_complex_element_float_input_on_captured_key_down(gui_element* element, gui_event* event, gui_application* app)
{       // UNFINSIHED
    if (!element || !event || !app || event->event_type != GUI_EVENT_TYPE_KEY_DOWN || !event->event_data)
        return;         // Sanity Check, i dont think this will ever run.

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_float_input_data* data = (gui_complex_element_float_input_data*)base_data->data;
    keypad_state key = *(keypad_state*)event->event_data;
    
    bool reset_recently_changed_magnitude = true;

    // Handle key inputs

    switch (key) 
    {
    case KEYPAD_STATE_BUTTON_ENT:
        gui_application_set_input_capture(app, NULL);
        return;
    case KEYPAD_STATE_BUTTON_PLUSMINUS:
        if (data->recently_changed_magnitude)
            data->current_magnitude *= -1;
        else
            data->current_coefficent *= -1;
        break;
    case KEYPAD_STATE_BUTTON_DEL:
        if (data->current_input_len == 0)
            return;     // Didn't do anything, dont redraw
        
        char last_input = data->current_str[--data->current_input_len];
        data->current_str[data->current_input_len] = '\0';
        
        if (last_input == '.')
        {
            data->decimal_multiplyer = 1;
            break;
        }

        float last_value = last_input - 0x30;

        if (data->decimal_multiplyer < 1)
        {
            data->decimal_multiplyer *= 10;
            last_value *= data->decimal_multiplyer;
        }
        
        if (data->current_coefficent < 0)
            last_value *= -1;

        data->current_coefficent -= last_value;

        if (data->decimal_multiplyer == 1)
            data->current_coefficent /= 10;
        
        if (data->current_coefficent == 0)
        {
            data->current_magnitude = data->allow_magnitude_change ? 0 : data->magntiude_at_default;
            data->current_input_len = 0;
        }
        break;
    case KEYPAD_STATE_BUTTON_CLR:
        data->current_magnitude = data->allow_magnitude_change ? 0 : data->magntiude_at_default;
        data->decimal_multiplyer = 1;
        data->current_coefficent = 0;
        data->current_input_len = 0;
        data->current_str[0] = '\0';
        break;
    case KEYPAD_STATE_BUTTON_0:
    case KEYPAD_STATE_BUTTON_1:
    case KEYPAD_STATE_BUTTON_2:
    case KEYPAD_STATE_BUTTON_3:
    case KEYPAD_STATE_BUTTON_4:
    case KEYPAD_STATE_BUTTON_5:
    case KEYPAD_STATE_BUTTON_6:
    case KEYPAD_STATE_BUTTON_7:
    case KEYPAD_STATE_BUTTON_8:
    case KEYPAD_STATE_BUTTON_9:
        if (data->current_input_len >= data->max_input_len)
            return;     // Dind't change anything dont redraw
        
        int digit = get_digiet_from_keypad_state(key);
        float digit_value = data->current_coefficent >= 0 ? digit : -digit;
        
        // This stops the werid issie where you end up it +0123
        if (data->current_coefficent == 0 && data->current_str[data->current_input_len - 1] == '0')
            data->current_input_len--;

        if (data->decimal_multiplyer == 1)
        {
            data->current_coefficent *= 10;
            data->current_coefficent += digit_value;
        }
        else 
        {
            data->current_coefficent += digit_value * data->decimal_multiplyer;
            data->decimal_multiplyer *= 0.1;
        } 

        data->current_str[data->current_input_len++] = (char)(0x30 + digit);
        break;
    case KEYPAD_STATE_BUTTON_DOT:
        if (data->current_input_len >= data->max_input_len || data->decimal_multiplyer != 1)
            return;         // Didn't change anything, dont redraw

        data->current_str[data->current_input_len++] = '.';
        data->decimal_multiplyer = 0.1;
        break;
    case KEYPAD_STATE_BUTTON_A:
    case KEYPAD_STATE_BUTTON_B:
    case KEYPAD_STATE_BUTTON_C:
        if (!data->allow_magnitude_change)
            return;         // No change required dont redraw

        data->current_magnitude = key == KEYPAD_STATE_BUTTON_A ? 2 : (key == KEYPAD_STATE_BUTTON_B ? 1 : 0);
        data->recently_changed_magnitude = true;
        reset_recently_changed_magnitude = false;
        break;
    default: // Did not change anything, we will ignore it completly
        return;
    } 
    
    if (reset_recently_changed_magnitude) 
        data->recently_changed_magnitude = false;
    
    draw_element(element, app->target_buffer);
}

void gui_complex_element_float_input_on_cursor_toggle(gui_element* element, gui_event* event, gui_application* app)
{
    if (!element || !app || !element->data)
        return;

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_float_input_data* data = (gui_complex_element_float_input_data*)base_data->data;

    data->text_data.cursor_visible = !data->text_data.cursor_visible;

    draw_element(element, app->target_buffer);
}

void gui_complex_element_float_input_on_capture_end(gui_element* element, gui_event* event, gui_application* app)
{
    if (!element || !app || !element->data)
        return;

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_float_input_data* data = (gui_complex_element_float_input_data*)base_data->data;
    
    data->output = data->current_coefficent * s_magnitudes[data->current_magnitude + 2];

    if (data->output < data->true_minimum)
    {
        data->decimal_multiplyer = data->decimal_multiplyer_at_minimum;
        data->current_magnitude = data->magntiude_at_minimum;
        data->current_coefficent = data->minimum_coefficent;
        strcpy_s(data->minimum_str, 32, data->current_str);
        data->current_input_len = strlen(data->current_str);
        data->output = data->true_minimum;
    }
    else if (data->output > data->true_maximum)
    {
        data->decimal_multiplyer = data->decimal_multiplyer_at_maximum;
        data->current_magnitude = data->magntiude_at_maximum;
        data->current_coefficent = data->maximum_coefficent;
        strcpy_s(data->maximum_str, 32, data->current_str);
        data->current_input_len = strlen(data->current_str);
        data->output = data->true_maximum;
    }
    else if (data->current_input_len == 0)
    {
        data->decimal_multiplyer = data->decimal_multiplyer_at_defult;
        data->current_magnitude = data->magntiude_at_default;
        data->current_coefficent = data->default_coefficent;
        strcpy_s(data->default_str, 32, data->current_str);
        data->current_input_len = strlen(data->current_str);
        data->output = data->true_minimum;
    }

    data->text_data.cursor_visible = false;

    draw_element(element, app->target_buffer);

    event = malloc(sizeof(gui_event));

    if (!event)
    {
        printf("[Error] Failed to create event for float input changed: malloc failed!");
        return;
    }

    memclr(event, sizeof(gui_event));
    event->event_type = GUI_EVENT_TYPE_INPUT_FEILD_FLOAT_CHANGED;
    event->event_data = element;

    gui_event_queue_push(&app->event_queue, event);
}

void gui_complex_element_float_input_draw_function(gui_element* element, gui_vec2 offset, int target_buffer)
{
    if (!element  || !element->data)
        return;

    gui_complex_element_data* base_data = (gui_complex_element_data*)element->data;
        
    if (!base_data->data) // Stop any weird errors
        return;

    gui_complex_element_float_input_data* data = (gui_complex_element_float_input_data*)base_data->data;

    char text_buffer[32 + 9];

    memclr(text_buffer, 32 + 9);
    strcpy_s(data->current_str, 32 + 8, text_buffer + 1);
    text_buffer[0] = data->current_coefficent >= 0 ? '+' : '-';
    
    // Only show suffix if needed 

    if (data->allow_magnitude_change || data->current_magnitude != 0)
    {

        const char** suffix_array = data->show_magnitude_as_sci ? s_scientific_notation : s_matrix_prefixes;
        strcat_s(text_buffer, 32 + 8, suffix_array[data->current_magnitude + 2]); 
    }
    
    // Draw unit
    if (data->unit)
        strcat_s(text_buffer, 32 + 8, data->unit);

    data->text_data.str = text_buffer;

    // Now (temporarlly) assign the text data as the element's data
    element->data = &data->text_data;
    
    gui_standard_element_textbox_draw_function(element, offset, target_buffer);

    element->data = base_data;
    data->text_data.str = NULL;
}


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
