#include "simple_waveform_input/input.h"

#include "gui/elements/complex.h"
#include "common/basic_io.h"
#include "common/math.h"

// TODO for the range inputs:
// TODO: split defult into its own system

void set_simple_waveform_input_amplitude_range(simple_waveform_input* input, float min, float max, int decimals)
{
    gui_complex_element_float_input_set_default(&input->amplitude_input, 2.5, 3);
    gui_complex_element_float_input_set_min_max(&input->amplitude_input, min, max, decimals);
}


void set_simple_waveform_input_freqency_range(simple_waveform_input* input, float min, float max, int decimals)
{
    gui_complex_element_float_input_set_default(&input->freqency_input, 1e3, 3);
    gui_complex_element_float_input_set_min_max(&input->freqency_input, min, max, decimals);
}


void set_simple_waveform_input_offset_range(simple_waveform_input* input, float min, float max, int decimals)
{
    gui_complex_element_float_input_set_default(&input->offset_input, 0, 3);
    gui_complex_element_float_input_set_min_max(&input->offset_input, min, max, decimals);
}

// This function will set where navigation inputs will go when a user 
// tries to naviagate out side the input element.
// @param input the input object to affect
// @param top The element the top will naviate to.
// @param bottom The element the bottom will naviagate to.
// @param left The element the left will naviagate to. 
// @param right The element the right will naviagate to.
// @note Like all navaiation settings, NULL will simply turn of navaiation to that side.
void set_simple_waveform_navigation_outputs(simple_waveform_input* input, gui_element* top, gui_element* bottom, 
        gui_element* left, gui_element* right);
