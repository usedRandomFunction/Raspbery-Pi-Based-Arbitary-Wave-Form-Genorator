#ifndef LIBGUI_APPLICATION_H
#define LIBGUI_APPLICATION_H

#include "gui/elements.h"
#include "gui/events.h"
#include "gui/timers.h"

#include "dynamic_array/dynamic_array.h"
#include "common/keypad.h"

#include <stdbool.h>

// stores all infomation about the application in a common format
struct gui_application
{
    gui_event_queue event_queue;                        // Self explanitory
    gui_timer_queue timer_queue;                        // Self explanitory, agian
    dynamic_array ui_elements;                          // Stores the UI elements, active or not (gui_element* array)
    keypad_state keypad;                                // Used internally to keep track of what keys are depressed
    int target_buffer;                                  // What frame buffer should it write to

    gui_element* current_navigation_selection;          // Stores a pointer to the current navigation selection
    bool navigation_enabled;                            // If true navigation is enabled
              
    gui_element* current_input_capture;                 // Stores a pointer to a complex gui elements which will capture input
                                                        // This is handled in gui_application_defult_event_handler, so users can block it.
                                                        // If this is NULL nothing will occur. It is still recomended to use the set input 
                                                        // capture function for this purpose.
};

typedef struct gui_application gui_application;

// Initializes the given application
// @param application Application to Initialize
void initialize_gui_application(gui_application* application);

// Frees the application, used durning shutdown
// @param application Application to free
// @note This will not attempt to free the gui_application header its self
void free_application(gui_application* application);

// Forces the GUI application to redraw all its elements
// @param application Application to redraw
void redraw_gui_application(gui_application* application);

// Gets the next event for the user app, similar to gui_event_queue_next, however,:
// it will also check for events (e.g button presses / timers) and add them to the queue.
// @note The function will halt execution while it waits for a event.
// @param application Application to get event for
// @return event to proccess or null, in which case the application should be closed
gui_event* gui_application_get_next_event(gui_application* application);

// Defult handler for diffrent events. e.g UI navigation using 8,4,2,6.
// @param application The application to handle this event as
// @param event The event to handle
void gui_application_defult_event_handler(gui_application* application, gui_event* event);

// Sets the navigation selection, and sends a NAV_FOCUS_CHANGED event
// also handles setting / resting the focused flag, and redrawing
// @param application The application to change the focus of
// @param selection The new selection to use
void gui_application_set_navigation_selection(gui_application* application, gui_element* selection);

// Sets the applications input capture. This is which (if any)
// element will have a function be ran for all key down / up events.
// This function will also create INPUT_CAPTURE_BEGIN and END events.
// @param application The application to change the input capture of.
// @param capture The element to capture as
// @note The element's data must be of type gui_complex_element_data.
// @note if NULL is passed the function will not atempt to capture with NULL,
//       but will simply remove the old capture 
void gui_application_set_input_capture(gui_application* application, gui_element* capture);

#endif
