#include "simple_waveform_input/input.h"

#include "gui/elements/standard.h"
#include "gui/elements/complex.h"
#include "common/basic_io.h"
#include "common/memory.h"

#define padding 3 
#define margin 5
#define floatinput_size_x_char 12 // TOOD auto calulate on the fly

// Initalizes the struct for simple_waveform_input, and creates all required elements 
// @param input The simple_waveform_input struct to Initalize 
// @param size_x The size of this input object in pixels
// @param size_y The size of this input object in pixels
void initalize_simple_waveform_input(simple_waveform_input* input, int size_x, int size_y)
{
    memclr(input, sizeof(simple_waveform_input));
    initialize_frame_element(&input->continer);
    input->continer.size.y = size_y;
    input->continer.size.x = size_x;

    input->continer.color.background = FRAMEBUFFER_RGB(0, 81, 156);     // #00519c
    input->continer.color.border = FRAMEBUFFER_RGB(193, 193, 193);      // #c1c1c1
    
    size_t my_ID = (size_t)input;
    input->continer.id = my_ID;

    gui_element* current_element = create_textbox_element("Wave type", padding, &input->continer.sub_elements);

    current_element->position.x = margin;
    current_element->position.y = margin;
    current_element->parent = &input->continer;

    gui_element* last_element = current_element;
    current_element = create_textbox_element("Freqency ", padding, &last_element->sub_elements);
    current_element->position.x = 0;
    current_element->position.y = margin + last_element->size.y;
    current_element->parent = last_element;
    

    gui_element* current_input = &input->freqency_input;
    initialize_float_input_element(current_input, padding, floatinput_size_x_char,
            false, true, "Hz");
    current_input->position.x = margin + current_element->size.x;
    current_input->color_focused.background = input->continer.color.background;
    current_input->color.background = input->continer.color.background;
    current_input->parent = current_element;
    current_input->id = my_ID + 1;
    insert_dynamic_array(&current_input, 0, &current_element->sub_elements); 

    
    last_element = current_element;

    current_element = create_textbox_element("Amplitude", padding, &last_element->sub_elements);
    current_element->position.x = 0;
    current_element->position.y = margin + last_element->size.y;
    current_element->parent = last_element;
    
    
    current_input = &input->amplitude_input;
    initialize_float_input_element(current_input, padding, floatinput_size_x_char,
            false, true, "V");
    current_input->position.x = margin + current_element->size.x;
    current_input->color_focused.background = input->continer.color.background;
    current_input->color.background = input->continer.color.background;
    current_input->parent = current_element;
    current_input->id = my_ID + 2;
    insert_dynamic_array(&current_input, 0, &current_element->sub_elements); 

    last_element = current_element;

    current_element = create_textbox_element("Offset   ", padding, &last_element->sub_elements);
    current_element->position.x = 0;
    current_element->position.y = margin + last_element->size.y;
    current_element->parent = last_element;

    current_input = &input->offset_input;
    initialize_float_input_element(&input->offset_input, padding, floatinput_size_x_char,
            false, true, "V");
    current_input->position.x = margin + current_element->size.x;
    current_input->color_focused.background = input->continer.color.background;
    current_input->color.background = input->continer.color.background;
    current_input->parent = current_element;
    current_input->id = my_ID + 2;
    insert_dynamic_array(&current_input, 0, &current_element->sub_elements);

    last_element = current_element;
    
    
    // TODO auto fill values based of config settings on the AWG
    set_simple_waveform_input_freqency_range(input, 1e-3, 5e6, 3);
    set_simple_waveform_input_amplitude_range(input, -5, 5, 3);
    set_simple_waveform_input_offset_range(input, -5, 5, 3);
    
    // Navigation controlls

    input->top                          = &input->freqency_input;
    input->freqency_input.nav.bottom    = &input->amplitude_input;

    input->amplitude_input.nav.top      = &input->freqency_input;
    input->amplitude_input.nav.bottom   = &input->offset_input;

    input->offset_input.nav.top         = &input->amplitude_input;
    input->bottom                       = &input->offset_input;


    // TOOD wave type selector
    // Sine
    // Triangle
    // Square
    // Noise ?

    // Slections
    // Normal:
    // Freqency
    // Amplitude
    // Offset
    
    // Extra for Triangle
    // sysemtry 

    // Extra for Square
    // Selector to use, duty, on time, off time 
    // 
}

void free_simple_waveform_input(simple_waveform_input* input) 
{
    // First clear any navigation inputs pointing to the object
    if (input->bottom->nav.bottom)
        input->bottom->nav.bottom->nav.top = NULL;
    if (input->top->nav.right)
        input->top->nav.right->nav.left = NULL;
    if (input->top->nav.left)
        input->top->nav.left->nav.right = NULL;
    if (input->top->nav.top)
        input->top->nav.top->nav.bottom = NULL;

    free_element_recursive(&input->continer);
}
