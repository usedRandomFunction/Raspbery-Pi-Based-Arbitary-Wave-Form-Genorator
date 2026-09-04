#include "simple_waveform_input/input.h"

#include "gui/elements/complex.h"
#include "common/basic_io.h"
#include "common/math.h"

// This returns the minimum requied number of samples to display the waveform, 
// such that the missmatch in phase at the end is <= to phase_missmatch
// i.e fmod(theta(Tmax)) <= phase_missmatch. Lower value more prescice but uses 
// more memory and takes longer to calcuate.
// @param input the input object to affect
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
