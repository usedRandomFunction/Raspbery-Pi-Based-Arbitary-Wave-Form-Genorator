#ifndef LIBGUI_ELEMENTS_COMPLEX
#define LIBGUI_ELEMENTS_COMPLEX

#include "gui/application.h"
#include "gui/elements.h"
#include "gui/events.h"


#include "common/keypad.h"



// Handles event based functions for complex gui elements.
// First parameter is the element to proccess the event as.
// Second parameter is the event to proccess.
// The Thrid parameter is the application the event came from
typedef void (*COMPLEX_ELEMENT_EVENT_HANDLER)(gui_element*, gui_event*, gui_application*);


// Holds function pointers for a element and a pointer to more data.
// @note If any function pointers are NULL when the application attempts to call 
//       the NULL function no error will occure and the call will simply be 
//       aborted.
struct gui_complex_element_data
{
    COMPLEX_ELEMENT_EVENT_HANDLER on_captured_key_down;      // Runs for every key down event while input is captrued by element
    COMPLEX_ELEMENT_EVENT_HANDLER on_captured_key_up;        // Runs for every key up event while input is captrued by element
    COMPLEX_ELEMENT_EVENT_HANDLER on_capture_begin;         // Runs when the element begins to capture input events
    COMPLEX_ELEMENT_EVENT_HANDLER on_capture_end;           // Runs when the element stops captruing input events

    void* data;                                               /* extra data (If required)*/
};
typedef struct gui_complex_element_data gui_complex_element_data;


struct gui_complex_element_integer_input_data
{
    bool recently_cleared;
    int64_t maximum;
    int64_t minimum;
    int64_t defult;
    int64_t value;

    gui_element_standard_element_text_box_data text_data;
};
typedef struct gui_complex_element_integer_input_data gui_complex_element_integer_input_data;


// Sets up the given element to be a integer_input
// @param element Element to set up
// @param padding Padding around text in pixels
// @param size_x_char The size of the element in characters
// @return 0 on success, and non-zero on error
int initialize_integer_input_element(gui_element* element, int padding, int size_x_char); 

// Allocates and initializes a integer_input element,
// with no background color, storing a pointer in buffer
// @param defult_value The Defult value to be used on start up or if the clr then enter is pressed
// @param minimum Minimum allowed value
// @param maximum Maximum allowed value
// @param padding Padding around text in pixels
// @param size_x_char The size of the element in characters
// @param buffer Buffer to add the pointer to the element to
// @return Pointer to the created element or NULL if failed
gui_element* create_integer_input_element(int64_t defult_value, int64_t minimum, int64_t maximum, int padding, int size_x_char, dynamic_array* buffer);

// Handles user input for "integer_input" elements
// @param element Element to handle input for
// @param event Event to handle
// @param app The application which the event belongs to
void gui_complex_element_integer_input_on_captured_key_down(gui_element* element, gui_event* event, gui_application* app);

// Ensures that values are with in allowed ranges, and will reset them if not.
// @param element Element to handle deselection of
// @param event Set to NULL; unused.
// @param app Pointer to application that deselection event is comething from.
void gui_complex_element_integer_input_on_capture_end(gui_element* element, gui_event* event, gui_application* app);

// Draw function for "integer input" elements
// @param element Element to draw
// @param offset The cumulative offset of the parent(s) of the element
// @param target_buffer Frame buffer to draw to
void gui_complex_element_integer_input_draw_function(gui_element* element, gui_vec2 offset, int target_buffer);


// TODO this one should definatly go in a seporate file, since it is not 
// a element function, but a helper used by a singler function

// Gets a signal digiet from the give keypad state.
// If more then one of the [0, 9] buttons are pressed
// the highest value will be reutrned
// @param state The keypad state to get the digiet from
// @return [0, 9] for the button, or -1 if no number keys are pressed
int get_digiet_from_keypad_state(keypad_state state); // TODO mabye move this to a diffrent include /input_help.h?

#endif
