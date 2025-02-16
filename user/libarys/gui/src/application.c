#include "gui/application.h"
#include "gui/elements.h"

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

// Handles UI navigation (8, 2, 4, 6 as arrows and enter)
// @param Application to handle as
// @param input Keypad input (from KEY_DOWN event)
static void s_handle_navigation_inputs(gui_application* application, keypad_state* input);

void initialize_gui_application(gui_application* application)
{
    memclr(application, sizeof(gui_application));

    initialize_gui_event_queue(&application->event_queue);
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

    free(&application->event_queue);
}

gui_event* gui_application_get_next_event(gui_application* application)
{
    gui_event* event = NULL;

    do 
    {
        s_add_keypad_events(application);
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
        if (application->navigation_enabled)
            s_handle_navigation_inputs(application, (keypad_state*)event->event_data);
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
        
        printf("[Error] Failed to create event for nav focus changed: malloc failed\n");
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

    gui_vec2 offset; // Used for redrawing
    memclr(&offset, sizeof(gui_vec2));

    if (application->current_navigation_selection)
    {
        application->current_navigation_selection->flags &= ~GUI_ELEMENT_FLAGS_FOCUSED;

        if (application->current_navigation_selection->draw)
            application->current_navigation_selection->draw(application->current_navigation_selection, offset, application->target_buffer);
    }

    if (selection)
    {
        selection->flags |= GUI_ELEMENT_FLAGS_FOCUSED;

        if (selection->draw)
            selection->draw(selection, offset, application->target_buffer);
    }

    application->current_navigation_selection = selection;
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

static void s_handle_navigation_inputs(gui_application* application, keypad_state* input)
{
    if (!application->navigation_enabled)
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