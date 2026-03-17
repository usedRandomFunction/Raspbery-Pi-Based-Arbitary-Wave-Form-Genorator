#ifndef LIBGUI_EVENTS_H
#define LIBGUI_EVENTS_H

#include "dynamic_array/dynamic_array.h"

// Stores the event type, and a pointer to heap allocated data (the queue will handler freeing), (or NULL depending on type)
struct gui_event
{
    int event_type;
    int event_metadata;     // Stores data about the event, struct not the event its self e.g can you delete event_data or not 
    void* event_data; 
};

typedef struct gui_event gui_event;

// Stores a first in first out queue, for the events
// this queue is heap allocated, and will grow if needed
// TODO Since the buffer can grow, it should probibly be able to shrink
struct gui_event_queue
{
    gui_event** buffer;
    int buffer_size;
    int write_index; 
    int read_index;
};

typedef struct gui_event_queue gui_event_queue;

// Frees event_data and the event its self
// @param event Event to free
// @warning This function will not free memory pointers stored within the event_data
void free_gui_event(gui_event* event);

// Initialize the fifo buffer
// @param queue Queue to initialize
void initialize_gui_event_queue(gui_event_queue* queue);

// Resizes the queue to store `number_of_entrys`
// @param queue Queue to resize
// @param number_of_entrys New size
// @note If resizing will delete entires the new size will be the minimum size to store all entires 
void resize_gui_event_queue(gui_event_queue* queue, int number_of_entrys); 

// Frees the buffer, and all events the buffer points to
// @param queue Queue to free
// @note This will not attempt to free the gui_event_queue header its self
void free_gui_event_queue(gui_event_queue* queue);

// Pushes a event to the back of the queue
// @param queue The queue to push onto
// @param event The event to push
// @note event Must be heap allocated, not on the stack, the pointer its self will be copyed to the queue, the memory will be freed later by the queue
void gui_event_queue_push(gui_event_queue* queue, gui_event* event);

// Gets the next event from the queue
// simultaneously frees the last event
// @param queue The queue to get the event from
// @return A pointer to the next event, or NULL if failed
gui_event* gui_event_queue_next(gui_event_queue* queue);

enum
{
    GUI_EVENT_TYPE_IGNORE                       = 0,    /* Event does nothing, ignore*/
    GUI_EVENT_TYPE_KEY_DOWN                     = 1,    /* Event triggerd for each button, which has started to be pressed. Event data is pointer to keypad_state */
    GUI_EVENT_TYPE_KEY_UP                       = 2,    /* Event triggerd for each button, which has stopped being pressed. Event data is pointer to keypad_state */
    GUI_EVENT_TYPE_REDRAW                       = 3,    /* If added to the queue redraw_gui_application is called by gui_application_defult_event_handler. No event data*/
    GUI_EVENT_TYPE_NAV_FOCUS_CHANGED            = 4,    /* Event triggered when the nav focus changes. Event data is pointer to gui_event_nav_focus_changed struct*/
    GUI_EVENT_TYPE_NAV_SELECT                   = 5,    /* Event triggered when enter is pressed and navigation is enabled. Event data is pointer to selected element */
    GUI_EVENT_TYPE_INPUT_CAPTURE_BEGIN          = 6,    /* Event triggered when a element begins to capture input. Event data is pointer to capturing element */
    GUI_EVENT_TYPE_INPUT_CAPTURE_END            = 7,    /* Event triggered when a element stops capturing input. Event data is pointer to element no longer capturing */
    GUI_EVENT_TYPE_INPUT_FEILD_INT_CHANGED      = 8,    /* Event triggered when a "gui_complex_element_integer_input"'s feild updates. Event data is pointer to element */
    GUI_EVENT_TYPE_TIMER_TRIGGERED              = 9,    /* Event triggered when a timer expires. Event data is pointer to gui_timer struct */
    GUI_EVENT_TYPE_INPUT_FEILD_FLOAT_CHANGED    = 10,   /* Event triggered when a "gui_complex_element_float_input"'s feild updates. Event data is pointer to element */
};

enum
{
    GUI_EVENT_METADATA_NONE = 0,                        /* Does nothing*/
    GUI_EVENT_METADATA_CAN_FREE_EVENT_DATA = (1 << 0),  /* If set event_data can be freed*/
};

struct gui_event_nav_focus_changed
{
    struct gui_element* previous_selection;
    struct gui_element* new_selection;
};

typedef struct gui_event_nav_focus_changed gui_event_nav_focus_changed;

#endif
