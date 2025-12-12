#include "gui/application.h"
#include "gui/elements.h"
#include "gui/elements/complex.h"

#include "gui/internal_timers.h"

#include "common/basic_io.h"
#include "common/memory.h"
#include "common/alloc.h"

// Creates any required keydown or up events for the applcaiton
// @param application Application to add events to
static void s_add_keypad_events(gui_application* application);

// Creates a event of type `type` for every key set to `1` in state
// @param application Application to add events to
// @param state Keystate to add events for
// @param type GUI_EVENT_TYPE enum
static void s_add_keypad_events_from_state(gui_application* application, keypad_state state, int type);

// Handles internal timers such as the cursor timer 
// @param Application to handle as
// @param timer The gui_timer to handle
static void s_handle_internal_gui_timers(gui_application* application, gui_timer* timer);
// Handles UI navigation (8, 2, 4, 6 as arrows and enter)
// @param Application to handle as
// @param input Keypad input (from KEY_DOWN event)
static void s_handle_navigation_inputs(gui_application* application, keypad_state* input);

// Creates an event for timers if needed
// @param application Application to add events to
static void s_add_timer_events(gui_application* application);

void initialize_gui_application(gui_application* application)
{
    memclr(application, sizeof(gui_application));

    initialize_gui_event_queue(&application->event_queue);
    initialize_gui_timer_queue(&application->timer_queue);
    initialize_dynamic_array(sizeof(gui_element*), 0, &application->ui_elements);


    gui_event* redraw_event = malloc(sizeof(gui_event));

    if (redraw_event)
    {
        memclr(redraw_event, sizeof(gui_event));
        redraw_event->event_type = GUI_EVENT_TYPE_REDRAW;

        gui_event_queue_push(&application->event_queue, redraw_event);
    }
    else
        printf("[Error] Unable to allocate memory for event\n");
}

void free_application(gui_application* application)
{
    if (!application)
        return;

    free_gui_event_queue(&application->event_queue);
}

gui_event* gui_application_get_next_event(gui_application* application)
{
    gui_event* event = NULL;

    do 
    {
        s_add_keypad_events(application);
        s_add_timer_events(application);
    }
    while (!(event = gui_event_queue_next(&application->event_queue)));

    return event;
}

void redraw_gui_application(gui_application* application)
{
    gui_element** buffer= (gui_element**)application->ui_elements.ptr;

    gui_vec2 offset;
    memclr(&offset, sizeof(gui_vec2));

    for (int i = 0; i < application->ui_elements.number_of_entrys; i++)
    {
        gui_element* element = buffer[i];

        if (element->flags & GUI_ELEMENT_FLAGS_HIDDEN)
            continue;               // Skip hidden elements

        draw_element_recursive(element, offset, application->target_buffer);
    }
}

void gui_application_defult_event_handler(gui_application* application, gui_event* event)
{
    if (!event)     // Handle NULL
        return;

    switch (event->event_type)
    {
    case GUI_EVENT_TYPE_REDRAW:
        redraw_gui_application(application);
        break;
    case GUI_EVENT_TYPE_KEY_DOWN:
    {
        if (application->navigation_enabled)
            s_handle_navigation_inputs(application, (keypad_state*)event->event_data);

        if (!application->current_input_capture)
            break;

        gui_complex_element_data* data = application->current_input_capture->data;

        if (data && data->on_captured_key_down)
            data->on_captured_key_down(application->current_input_capture, event, application);

        break;
    }
    case GUI_EVENT_TYPE_KEY_UP: // This probilit should be its own funciton
    {
        if (!application->current_input_capture)
            break;

        gui_complex_element_data* data = application->current_input_capture->data;

        if (data && data->on_captured_key_up)
            data->on_captured_key_up(application->current_input_capture, event, application);

        break;
    }
    case GUI_EVENT_TYPE_NAV_SELECT:
    {
        gui_element* element = (gui_element*)event->event_data;

        if (element && element->flags & GUI_ELEMENT_FLAGS_CAN_CAPTURE_INPUT && element != application->last_input_capture)
            gui_application_set_input_capture(application, (gui_element*)event->event_data);
        
        application->last_input_capture = NULL;

        break;
    }
    case GUI_EVENT_TYPE_TIMER_TRIGGERED:
        s_handle_internal_gui_timers(application, (gui_timer*)event->event_data);
        break;
    default:
        break;
    }
}

void gui_application_set_navigation_selection(gui_application* application, struct gui_element* selection)
{
    if (!application)
        return;

    gui_event_nav_focus_changed* event_data = malloc(sizeof(gui_event_nav_focus_changed));
    gui_event* event = malloc(sizeof(gui_event));
    
    if (event_data == NULL || event == NULL)
    {
        if (event_data)
            free(event_data);
        
        if (event)
            free(event);
        
        printf("[Error] Failed to create event for nav focus changed: malloc failed.\n");
    }
    else
    {
        memclr(event_data, sizeof(gui_event_nav_focus_changed));
        memclr(event, sizeof(gui_event));

        event->event_metadata = GUI_EVENT_METADATA_CAN_FREE_EVENT_DATA;
        event->event_type = GUI_EVENT_TYPE_NAV_FOCUS_CHANGED;
        event->event_data = event_data;

        event_data->previous_selection = application->current_navigation_selection;
        event_data->new_selection = selection;

        gui_event_queue_push(&application->event_queue, event);
    }


    if (application->current_navigation_selection)
    {
        application->current_navigation_selection->flags &= ~GUI_ELEMENT_FLAGS_FOCUSED;

        draw_element(application->current_navigation_selection, application->target_buffer);
    }

    if (selection)
    {
        selection->flags |= GUI_ELEMENT_FLAGS_FOCUSED;

        draw_element(selection, application->target_buffer);
    }

    application->current_navigation_selection = selection;
}


void gui_application_set_input_capture(gui_application* application, struct gui_element* capture)
{
    if (!application)
        return;

    // First create event for the end of the current capture (If it exists)
    // We also call its on end function
    if (application->current_input_capture)
    {
        gui_event* event = malloc(sizeof(gui_event));

        if (!event)
        {
            printf("[Error] Failed to create event for INPUT_CAPTURE_END: malloc failed.\n");
        }
        else 
        {
            memclr(event, sizeof(gui_event));
            event->event_data = application->current_input_capture;
            event->event_type = GUI_EVENT_TYPE_INPUT_CAPTURE_END;

            gui_event_queue_push(&application->event_queue, event);
        }

        gui_complex_element_data* data = application->current_input_capture->data;

        if (data && data->on_capture_end)
            data->on_capture_end(application->current_input_capture, NULL, application);
    }

    // Now a event for the new capture (If it exists)
    // And we call its beign function
    if (capture)
    {
        gui_event* event = malloc(sizeof(gui_event));

        if (!event)
        {
            printf("[Error] Failed to create event for INPUT_CAPTURE_BEGIN: malloc failed.\n");
        }
        else 
        {
            memclr(event, sizeof(gui_event));
            event->event_data = capture;
            event->event_type = GUI_EVENT_TYPE_INPUT_CAPTURE_BEGIN;

            gui_event_queue_push(&application->event_queue, event);
        }

        gui_complex_element_data* data = capture->data;

        if (data)
        {
            if (data->on_capture_begin)
                data->on_capture_begin(application->current_input_capture, NULL, application);
            
            if (data->on_cursor_toggle)
                gui_timer_queue_create_future_timer(&application->timer_queue, 
                                                    GUI_INTERNAL_TIMER_ID_CURSOR_DELAY, 
                                                    GUI_INTERNAL_TIMER_ID_CURSOR);
        }

    }

    application->last_input_capture = application->current_input_capture;
    application->current_input_capture = capture;
}




static void s_add_keypad_events(gui_application* application)
{
    const keypad_state new_state = get_keypad_state();
    const keypad_state delta = application->keypad ^ new_state;
    
    if (!delta)     // No new key presses?, do nothing
        return;
    
    
    const keypad_state keydown = delta & new_state;
    const keypad_state keyup = delta & application->keypad;
    application->keypad = new_state;

    s_add_keypad_events_from_state(application, keydown, GUI_EVENT_TYPE_KEY_DOWN);
    s_add_keypad_events_from_state(application, keyup, GUI_EVENT_TYPE_KEY_UP);
}

static void s_add_keypad_events_from_state(gui_application* application, keypad_state state, int type)
{
    if (!state)     
        return;                 // No buttons in state? do nothing
    
    // Loop over every button
    for (keypad_state i = 1; i != 0; i <<= 1)
    {
        if (!(state & i))
            continue;           // Button not included in state? do nothing
            
        keypad_state* event_data = malloc(sizeof(keypad_state));
        gui_event* event = malloc(sizeof(gui_event));
        
        if (event_data == NULL || event == NULL)
        {
            if (event_data)
                free(event_data);
            
            if (event)
                free(event);
            
            printf("[Error] Failed to create event for keypress: malloc failed\n");
            continue;
        }
        
        memclr(event, sizeof(gui_event));
        *event_data = i;
        event->event_metadata = GUI_EVENT_METADATA_CAN_FREE_EVENT_DATA;
        event->event_data = event_data;
        event->event_type = type;
        
        gui_event_queue_push(&application->event_queue, event);
    }
}

static void s_handle_internal_gui_timers(gui_application* application, gui_timer* timer)
{
    if (!application || !timer) // Missing data? do nothing
        return;

    switch (timer->id) 
    {
    case GUI_INTERNAL_TIMER_ID_CURSOR:
    {
        gui_element* active_capture = application->current_input_capture;

        if (!active_capture)
            break;

        gui_complex_element_data* data = active_capture->data;

        if (!data || !data->on_cursor_toggle) // Only do something if the function can handle it
            break;
        
        data->on_cursor_toggle(active_capture, NULL, application);
        gui_timer_queue_create_future_timer(&application->timer_queue, 
                                            GUI_INTERNAL_TIMER_ID_CURSOR_DELAY, 
                                            GUI_INTERNAL_TIMER_ID_CURSOR);

        break;
    }
    default:
        break;
    }
}

static void s_handle_navigation_inputs(gui_application* application, keypad_state* input)
{
    if (!application || !application->navigation_enabled)
        return;

    gui_element* current_selection = application->current_navigation_selection;
    gui_navigation* current_nav_options = &current_selection->nav;
    gui_element* new_selection = NULL;

    switch (*input)
    {
    case KEYPAD_STATE_BUTTON_ENT:
        {
            gui_event* event = malloc(sizeof(gui_event));
        
            if (event == NULL)
            {
                printf("[Error] Failed to create event for nav select: malloc failed\n");
                return;
            }
            memclr(event, sizeof(gui_event));

            event->event_data = application->current_navigation_selection;
            event->event_type = GUI_EVENT_TYPE_NAV_SELECT;
            gui_event_queue_push(&application->event_queue, event);
        return;
        }
    case KEYPAD_STATE_BUTTON_8:
        new_selection = current_nav_options->top;
        break;
    case KEYPAD_STATE_BUTTON_2:
        new_selection = current_nav_options->bottom;
        break;
    case KEYPAD_STATE_BUTTON_4:
        new_selection = current_nav_options->left;
        break;
    case KEYPAD_STATE_BUTTON_6:
        new_selection = current_nav_options->right;
        break;
    default:
        return;
    }

    if (new_selection == NULL)      // I.E return if no nav input or invalid destanation
        return;

    if (new_selection->flags & GUI_ELEMENT_FLAGS_DISABLED && !(current_selection->flags & GUI_ELEMENT_FLAGS_DISABLED))
        return;     // Dont change selection if the destination is dissabled, unless the current location is also disabled

    gui_application_set_navigation_selection(application, new_selection);
}

static void s_add_timer_events(gui_application* application) 
{
    gui_event* event = gui_timer_check_for_timer_events(&application->timer_queue);

    if (event)
        gui_event_queue_push(&application->event_queue, event);
}
