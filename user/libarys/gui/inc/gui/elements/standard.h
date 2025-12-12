#ifndef GUI_ELEMENTS_STANDARD_H
#define GUI_ELEMENTS_STANDARD_H

#include "gui/elements.h"

struct gui_standard_element_text_box_data // TODO rename so its just gui_standard_element_text_box_data
{
    bool cursor_visible;    // If the text has space left in the box the forground color will be placed on char wide if tree
    gui_vec2 offset;        // The offset from the top left, where display_draw_string is called
    const char* str;        // Text to draw
};

// Stores a pointer to the text and the offset
typedef struct gui_standard_element_text_box_data gui_standard_element_text_box_data;


// Sets up the given element to be a "Frame"
// A Frame is used can as a container, or as a background
// @param element Element to set up
void initialize_frame_element(gui_element* element);

// Sets up the given element to be a textbox
// @param element Element to set up
void initialize_textbox_element(gui_element* element);

// Sizes the given textbox so the edge of the background is `padding` px away from the text
// @param element textbox to size
// @param padding Padding in pixelis
void size_textbox_element(gui_element* element, int padding);

// Sizes the given textbox so the edge of the background is `padding` px away when n 
// characters of text are drawn
// @param element textbox to size
// @param number of characters to size for
// @param padding Padding in pixelis
void size_textbox_element_for_n_characters(gui_element *element, int n, int padding);

// Moves the given text / textbox element so its center is at the current possition
// @param element text / textbox to possition
// @param horizontal If true the element will be centered horizontally, If false x will not be changed
// @param vertical Uf true the element will be centered vertical, If false y will not be changed
void center_text_element(gui_element* element, bool horizontal, bool vertical);

// Aligns the given text to the right
// @param element text / textbox to possition
void right_allign_text_element(gui_element* element);

// Allocates and initializes a frame element,
// storing a pointer in buffer
// @param buffer Buffer to add the pointer to the element to
// @return Pointer to the created element or NULL if failed
gui_element* create_frame_element(dynamic_array* buffer);

// Allocates and initializes a textbox element,
// with no background color, storing a pointer in buffer
// @param text, Text to show on the element
// @param buffer Buffer to add the pointer to the element to
// @return Pointer to the created element or NULL if failed
gui_element* create_text_element(const char* text, dynamic_array* buffer);

// Allocates and initializes a textbox element,
// storing a pointer in buffer
// @param text, Text to show on the element
// @param padding Padding in pixels
// @param buffer Buffer to add the pointer to the element to
// @return Pointer to the created element or NULL if failed
gui_element* create_textbox_element(const char* text, int padding, dynamic_array* buffer);

// Draw function for "Frame" elements
// @param element Element to draw
// @param offset The cumulative offset of the parent(s) of the element
// @param target_buffer Frame buffer to draw to
void gui_standard_element_frame_draw_function(gui_element* element, gui_vec2 offset, int target_buffer);

// Draw function for "text", "textbox", "textbox_autosized" elements
// @param element Element to draw
// @param offset The cumulative offset of the parent(s) of the element
// @param target_buffer Frame buffer to draw to
void gui_standard_element_textbox_draw_function(gui_element* element, gui_vec2 offset, int target_buffer);

#endif
