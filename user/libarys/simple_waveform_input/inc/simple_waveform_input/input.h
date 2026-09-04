#ifndef LIBSIMPLE_WAVEFORM_INPUT_INPUT_H
#define LIBSIMPLE_WAVEFORM_INPUT_INPUT_H

#include "gui/elements.h"
#include "gui/events.h"


// This one struct contains all infomation required to make the input work
// @note To simply the code the ID of the elements will be based of the pointer address so, Dont Move It.
struct simple_waveform_input
{
    gui_element continer;       // This is the parent element to the entire object. [Also draws the background]
    gui_element* bottom;        // Can be used by other elements so nav in / out will work
    gui_element* top;           // Can be used by other elements so nav in / out will work
    gui_element freqency_input;
    gui_element amplitude_input;
    gui_element offset_input;
};

typedef struct simple_waveform_input simple_waveform_input;


// Initalizes the struct for simple_waveform_input, and creates all required elements 
// @param input The simple_waveform_input struct to Initalize 
// @param size_x The size of this input object in pixels
// @param size_y The size of this input object in pixels
void initalize_simple_waveform_input(simple_waveform_input* input, int size_x, int size_y);

// Frees all elements assoicated with the input object.
// @param input The input to free 
// @note This will not run free(input); If that is needed the called needs to do it them self.
void free_simple_waveform_input(simple_waveform_input* input);

// Sets the allowed amplitude range.
// @param input the input object to affect
// @param min The minimum of the allowed range 
// @param max THe maximum of the allowed range let g:c_syntax_for_h = 1
// @param decimals The number of decimal places to passed to the float input window 
void set_simple_waveform_input_amplitude_range(simple_waveform_input* input, float min, float max, int decimals);

// Sets the allowed freqency range.
// @param input the input object to affect
// @param min The minimum of the allowed range 
// @param max THe maximum of the allowed range 
// @param decimals The number of decimal places to passed to the float input window 
void set_simple_waveform_input_freqency_range(simple_waveform_input* input, float min, float max, int decimals);

// Sets the allowed offset range.
// @param input the input object to affect
// @param min The minimum of the allowed range 
// @param max THe maximum of the allowed range 
// @param decimals The number of decimal places to passed to the float input window 
void set_simple_waveform_input_offset_range(simple_waveform_input* input, float min, float max, int decimals);

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

// @param input the input object to affect
// This returns the minimum requied number of samples to display the waveform, 
// such that the missmatch in phase at the end is <= to phase_missmatch
// i.e fmod(theta(Tmax)) <= phase_missmatch. Lower value more prescice but uses 
// more memory and takes longer to calcuate.
// @param phase_missmatch The maximum allowed phase_missmatch [Radians]
//                        [A negitive number will allow any missmatch.
//                         This will return (int)(sample_rate / freqency)] 
// @return The minimum number of samples requied to do the above at the curent DAC sample_rate.
int get_number_of_samples_required(simple_waveform_input* input, float phase_missmatch);


// This calcuates the value of the waveform at the given time.
// @param input the input object to affect
// @param time This offset in seconds from the start of the waveform
// @return The voltage at the given time
float get_simple_at_time(simple_waveform_input* input, float time);

// To be placed in the event_handler function of the app, 
// without this in input backend simply wont update.
// @param event the gui_event to handle
// @return Pointer to the updated input, NULL if no update
simple_waveform_input* event_handler_simple_waveform_input(gui_event* event);

#endif
