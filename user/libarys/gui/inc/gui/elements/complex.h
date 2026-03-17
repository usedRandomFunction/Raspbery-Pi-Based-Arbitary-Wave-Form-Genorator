#ifndef LIBGUI_ELEMENTS_COMPLEX_H
#define LIBGUI_ELEMENTS_COMPLEX_H

#include "gui/elements/standard.h"
#include "gui/application.h"
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
    COMPLEX_ELEMENT_EVENT_HANDLER on_captured_key_down;     // Runs for every key down event while input is captrued by element
    COMPLEX_ELEMENT_EVENT_HANDLER on_captured_key_up;       // Runs for every key up event while input is captrued by element
    COMPLEX_ELEMENT_EVENT_HANDLER on_capture_begin;         // Runs when the element begins to capture input events
    COMPLEX_ELEMENT_EVENT_HANDLER on_cursor_toggle;         // Runs when the cursor's state is toggled. (Runs every 500 ms, only when captured)
    COMPLEX_ELEMENT_EVENT_HANDLER on_capture_end;           // Runs when the element stops captruing input events

    void* data;                                               /* extra data (If required)*/
};
typedef struct gui_complex_element_data gui_complex_element_data;


// Holds tje infomation requied to make the integer input work
struct gui_complex_element_integer_input_data
{
    bool recently_cleared;                          // Internal value. Uesd to determin if the value should 
                                                    // be reset to default when ent is pressed  
    int64_t maximum;                                // The maximum value allowed. Will be foceablity set to max if input excedes
    int64_t minimum;                                // The minimum value allowed. Will be foceablity set to min if input excedes
    int64_t default_v;                              // The default value, if nothing is entered will be reset to this.
    int64_t value;                                  // The current value

    gui_standard_element_text_box_data text_data;   // Text box, used to display
};
typedef struct gui_complex_element_integer_input_data gui_complex_element_integer_input_data;


// Stores all the data that will be needed for floating point input 
// This include the magnitude setting
struct gui_complex_element_float_input_data 
{
    bool allow_magnitude_change;                    // If False the A B C buttons will not change the magnitude of the input
    bool show_magnitude_as_sci;                     // If False shows magnitude as SI prefix, else 10^{N}
    bool recently_changed_magnitude;                // Internal value used to +- can change magnitude when needed
    
    int8_t current_input_len;                       // The current length of the input
    int8_t max_input_len;                           // The maximum number of chars the input is allowed to be

    int8_t magntiude_at_default;                    // magnitude setting for default value
    int8_t magntiude_at_maximum;                    // magnitude setting for maximum value
    int8_t magntiude_at_minimum;                    // magnitude setting for minimum value
    int8_t current_magnitude;                       // magnitude setting for current input

    float maximum_coefficent;                       // coefficent for maximum value
    float minimum_coefficent;                       // coefficent for minimum value
    float default_coefficent;                       // coefficent for default value
    float current_coefficent;                       // coefficent for the current inpit
    

    float true_maximum;                             // maximum_coefficent * 10 ^ [3 * magntiude_at_maximum]
    float true_minimum;                             // minimum_coefficent * 10 ^ [3 * magntiude_at_minimum]
    float true_default;                              // default_coefficent * 10 ^ [ 3* magntiude_at_default]
    float output;                                   // Output value, current * 10 ^ [3 * current_magnitude]
    
    float decimal_multiplyer_at_maximum;            // Ensures that the decimal point works properly for these states
    float decimal_multiplyer_at_minimum;            //
    float decimal_multiplyer_at_defult;             //
    float decimal_multiplyer;                       // Internal value used to help the decimal point work

    const char* maximum_str;                        // Display string [suffix not included] for maximum 
    const char* minimum_str;                        // Display string [suffix not included] for minimum 
    const char* default_str;                        // Display string [suffix not included] for default 
    const char* suffix;                             // suffix currently in use for input.
    const char* unit;                               // Stores a string for the unit, NULL means dont sitplay

    char current_str[32];                           // The string buffer for the current coefficent
    
    gui_standard_element_text_box_data text_data;   // Text box, used to display

};
typedef struct gui_complex_element_float_input_data gui_complex_element_float_input_data;

// Sets up the given element to be a integer_input
// @param element Element to set up
// @param padding Padding around text in pixels
// @param size_x_char The size of the element in characters
// @return 0 on success, and non-zero on error
int initialize_integer_input_element(gui_element* element, int padding, int size_x_char); 

// Allocates and initializes a integer_input element,
// with no background color, storing a pointer in buffer
// @param default_value The default value to be used on start up or if the clr then enter is pressed
// @param minimum Minimum allowed value
// @param maximum Maximum allowed value
// @param padding Padding around text in pixels
// @param size_x_char The size of the element in characters
// @param buffer Buffer to add the pointer to the element to
// @return Pointer to the created element or NULL if failed
gui_element* create_integer_input_element(int64_t default_value, int64_t minimum, int64_t maximum, int padding, int size_x_char, dynamic_array* buffer);

// Handles user input for "integer_input" elements
// @param element Element to handle input for
// @param event Event to handle
// @param app The application which the event belongs to
void gui_complex_element_integer_input_on_captured_key_down(gui_element* element, gui_event* event, gui_application* app);

// Toggles cursor_visible... and redraws 
// @param element Element to toggle the cursor on
// @param event Set to NULL; unused.
// @param app Pointer to application that cursor event is comething from.
void gui_complex_element_integer_input_on_cursor_toggle(gui_element* element, gui_event* event, gui_application* app);

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

// Sets up the given element to be a float_input
// @param element Element to set up
// @param padding Padding around text in pixels
// @param size_x_char The size of the element in characters
// @return 0 on success, and non-zero on error
int initialize_float_input_element(gui_element* element, int padding, int size_x_char); 

// Allocates and initializes a float_input element,
// with no background color, storing a pointer in buffer
// @param show_magnitude_as_sci If False the A B C buttons will not change the magnitude of the input
// @param allow_magnitude_change If False shows magnitude as SI prefix, else 10^{N}
// @param default_coefficent The default value to be used on start up or if the clr then enter is pressed
// @param default_magnitude The magnitude of the default value
// @param default_string The string represnetation of the default coefficent
// @param decimal_multiplyer_at_defult if the coefficent is not a integer this is 0.{N * 0} 1 where  N is the number id decimals
// @param maximum_coefficent coefficent of the maximum allowed value
// @param maximum_magnitide The magnitude of the maximum allowed value
// @param maximum_string The string represnetation of the maximum coefficent
// @param decimal_multiplyer_at_maximum if the coefficent is not a integer this is 0.{N * 0} 1 where  N is the number id decimals
// @param minimum_coefficent coefficent of the minimum allowed value
// @param minimum_magnitide The magnitude of the minimum allowed value
// @param minimum_string The string represnetation of the minimum coefficent
// @param decimal_multiplyer_at_minimum if the coefficent is not a integer this is 0.{N * 0} 1 where  N is the number id decimals
// @param unit The unit / final suffix for the string. NULL means dont display
// @param padding Padding around text in pixels
// @param size_x_char The size of the element in characters
// @param buffer Buffer to add the pointer to the element to
// @return Pointer to the created element or NULL if failed
// @note Magnitudes are only vaild for [-2, +2]. They are used as 10 ^ 3M
gui_element* create_float_input_element(bool show_magnitude_as_sci, bool allow_magnitude_change,
                                        float default_coefficent, uint8_t default_magnitude, const char* default_string,
                                        float decimal_multiplyer_at_defult,
                                        float maximum_coefficent, uint8_t maximum_magnitide, const char* maximum_string, 
                                        float decimal_multiplyer_at_maximum,
                                        float minimum_coefficent, uint8_t minimum_magnitide, const char* minimum_string,
                                        float decimal_multiplyer_at_minimum,
                                        const char* unit, int padding, int size_x_char, dynamic_array* buffer);

// Handles user input for "float_input" elements
// @param element Element to handle input for
// @param event Event to handle
// @param app The application which the event belongs to
void gui_complex_element_float_input_on_captured_key_down(gui_element* element, gui_event* event, gui_application* app);

// Toggles cursor_visible... and redraws 
// @param element Element to toggle the cursor on
// @param event Set to NULL; unused.
// @param app Pointer to application that cursor event is comething from.
void gui_complex_element_float_input_on_cursor_toggle(gui_element* element, gui_event* event, gui_application* app);

// Ensures that values are with in allowed ranges, and will reset them if not.
// @param element Element to handle deselection of
// @param event Set to NULL; unused.
// @param app Pointer to application that deselection event is comething from.
void gui_complex_element_float_input_on_capture_end(gui_element* element, gui_event* event, gui_application* app);

// Draw function for "float input" elements
// @param element Element to draw
// @param offset The cumulative offset of the parent(s) of the element
// @param target_buffer Frame buffer to draw to
void gui_complex_element_float_input_draw_function(gui_element* element, gui_vec2 offset, int target_buffer);

// TODO this one should definatly go in a seporate file, since it is not 
// a element function, but a helper used by a singler function

// Gets a signal digiet from the give keypad state.
// If more then one of the [0, 9] buttons are pressed
// the highest value will be reutrned
// @param state The keypad state to get the digiet from
// @return [0, 9] for the button, or -1 if no number keys are pressed
int get_digiet_from_keypad_state(keypad_state state); // TODO mabye move this to a diffrent include /input_help.h?

#endif
