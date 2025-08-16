#ifndef LIBGUI_ELEMENTS_H
#define LIBGUI_ELEMENTS_H

#include "dynamic_array/dynamic_array.h"
#include "common/display.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct gui_vec2
{
    uint32_t x;
    uint32_t y;
};

// Exists for organizational purpases
typedef struct gui_vec2 gui_vec2;

struct gui_element_colors
{
    display_color forground;
    display_color background;
    display_color border;
};

// Stores colors in one place
typedef struct gui_element_colors gui_element_colors;

struct gui_navigation
{
    struct gui_element* top;
    struct gui_element* bottom;
    struct gui_element* left;
    struct gui_element* right;
};

// Stores where navigation options lead
typedef struct gui_navigation gui_navigation;

// These functions take the target element and draw it, not including sub_elements, the int is the target buffer
typedef void (*DRAW_FUNCTION_POINTER)(struct gui_element*, gui_vec2, int);

struct gui_element
{

    gui_vec2 position;
    gui_vec2 size;
    int border_width_focused;
    int border_width_normal;

    gui_element_colors color_focused;
    gui_element_colors color_disabled;
    gui_element_colors color;

    gui_navigation nav;

    dynamic_array sub_elements;         // Array of gui_element pointers

    int flags;
    
    size_t id;                          // Does not have to be set, but can be useful when determening what element a event came from

    void* data;                         // Pointer to extra data (If required)

    struct gui_element* parent;                
    DRAW_FUNCTION_POINTER draw;         // Draws the element
};

// Holds all standered data about a element
typedef struct gui_element gui_element;


// Draws the given element (Ignoring the hidden flag), and all sub elements (using hidden flag)
// This function is basicly a wrapper around draw_element_recursive, but it uses the parent(s)
// Find the offset for you
// @param element Element to draw
// @param target_buffer Target frame buffer
void draw_element(gui_element* element, int target_buffer);

// Takes the given element, and draws it (ignoring hidden flag), then draws all sub elements (using hidden flag)
// @param element Element to draw
// @param offset The cumulative offset of the parent(s) of the element
// @param target_buffer Target frame buffer
void draw_element_recursive(gui_element* element, gui_vec2 offset, int target_buffer);

// Uses GUI_ELEMENT_FLAGS to determine the correct color
// @param element Element to get the color of
// @return Pointer to either element->color, element->color_focused or element->color_disabled
gui_element_colors* get_element_colors(gui_element* element);

// Finds the index of the given element in the given buffer
// @param element Element to find
// @param buffer Buffer to search in
// @return Index of element or -1 if failed
size_t find_element_index(gui_element* element, dynamic_array* buffer);

// Frees the element, and all contained pointers, as well as the subelements
// @param element Element to free
void free_element_recursive(gui_element* element);

// Frees memory assicated with all sub elements, and their elements...
// @param element Element to free the sub elements of
// @note This function does not free the element its self
void free_sub_elements_recursive(gui_element* element);

// Removes the given element from the buffer, and then 
// frees all sub elements, and their elements 
// followed by the element its self
// @param Element to free
// @param buffer Buffer to remove pointer to element form
void remove_and_free_element_recursive(gui_element* element, dynamic_array* buffer);

// Gives defult values and initializes sub_elements
// @param element Element to initialize
void initialize_standered_element_values(gui_element* element);

// Allocates and zeros memory for a event, 
// and stores it at the end of the given buffer
// @param buffer Buffer to add the pointer to the element to
// @return Pointer to the created element or NULL if failed
gui_element* create_element(dynamic_array* buffer);

// Possitions the current element virticaly beneath (or above) the last element with padding px, between
// @param element Element to possition
// @param padding The distance between elements in px, if positive `element` is placed beneath last_element, if negitive, it is above, 
// @param last_element The element virticaly above (or beneath) the current element
// @note A padding of -1 is interpreted as 0 px above, -2 is 1 px above...
// @return The pointer element points to
gui_element* possition_element_vertically(gui_element* element, int padding, gui_element* last_element);

// Possitions the current element horizontally right (or left) the last element with padding px, between
// @param element Element to possition
// @param padding The distance between elements in px, if positive `element` is placed to the right last_element, if negitive, it is to the left, 
// @param last_element The element virticaly above (or beneath) the current element
// @note A padding of -1 is interpreted as 0 px left, -2 is 1 px left...
// @return The pointer element points to
gui_element* possition_element_horizontally(gui_element* element, int padding, gui_element* last_element);


enum
{
    GUI_ELEMENT_FLAGS_NONE = 0,                         /* Does nothing. */
    GUI_ELEMENT_FLAGS_FOCUSED = (1 << 0),               /* Set by nav controlls, if set the element is focused*/
    GUI_ELEMENT_FLAGS_HIDDEN = (1 << 1),                /* If set the element will not be drawn. */
    GUI_ELEMENT_FLAGS_DISABLED = (1 << 2),              /* If set the element can not be focused / selected.*/
    GUI_ELEMENT_FLAGS_CAN_CAPTURE_INPUT = (1 << 3),     /* If set the element can begin to capture inputs when enter*/
                                                        /* is pressed while it is focused and navigation mode is*/
                                                        /* enabled. This operation will disabled navigation mode.*/
                                                        /* The input is captured in gui_application_defult_event_handler, */
                                                        /* The input is captured in gui_application_defult_event_handler, */
                                                        /* So a user can still block it if required. */
                                                        /* The element's data must be of type gui_complex_element_data. */
};

#endif
