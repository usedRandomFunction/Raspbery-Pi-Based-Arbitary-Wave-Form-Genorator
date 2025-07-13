#include "gui/elements/complex.h"
#include "common/basic_io.h"
#include "common/memory.h"
#include "common/alloc.h"


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
    complex_data->on_capture_end = gui_complex_element_integer_input_on_capture_end;
    complex_data->on_capture_begin = NULL;

    gui_complex_element_integer_input_data* data = malloc(sizeof(gui_complex_element_integer_input_data));

    if (data == NULL)
    {
        printf("[Error] Failed to initialize intieger input element, malloc failed\n");
        free(complex_data);
        return 2;
    }

    memclr(data, sizeof(gui_complex_element_integer_input_data));
    gui_element_standard_element_text_box_data* text_data = &data->text_data;
    element->data = text_data;
    size_textbox_element_for_n_characters(element, size_x_char, padding);
    
    


    element->flags |= GUI_ELEMENT_FLAGS_CAN_CAPTURE_INPUT;
    element->data = complex_data;
    complex_data->data = data;

    return 0;
}


gui_element* create_integer_input_element(int64_t defult_value, int64_t minimum, int64_t maximum, int padding, int size_x_char, dynamic_array* buffer)
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

    data->defult = defult_value;
    data->value = defult_value;
    data->minimum = minimum;
    data->maximum = maximum;
    data->recently_cleared = false;

    return element;
}

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
        data->value = data->defult;

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
