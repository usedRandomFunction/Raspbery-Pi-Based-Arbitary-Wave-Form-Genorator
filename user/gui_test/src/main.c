#include "gui/application.h"
#include "gui/elements.h"

#include "common/basic_io.h"
#include "common/display.h"
#include "common/memory.h"
#include "common/alloc.h"

gui_application app;

// Handles events for the app
// @param event Pointer to current event
void event_handler(gui_event* event);

// Handler for NAV_FOCUS_CHANGED events
// @param event pointer to event
void on_nav_focus_changed(gui_event* event);

// Handler for NAV_SELECT events
// @param event pointer to event
void on_nav_select(gui_event* event);

int main()
{
    initialize_gui_application(&app);

    gui_element* current_element = create_frame_element(&app.ui_elements);
    current_element->size.x = get_display_width() - 1;
    current_element->size.y = get_display_height() - 1;
    current_element->flags |= GUI_ELEMENT_FLAGS_DISABLED;

    current_element = create_textbox_element("Begin List", 3, &app.ui_elements);
    current_element->position.y = 100;
    current_element->position.x = 100;

    gui_element* first_element = NULL;
    gui_element* last_element = current_element;

    for (int i = 0; i < 10; i++)
    {
        char* buffer = malloc(7);
        memcpy(buffer, "testN!", 7);

        buffer[4] = (char)(i + 0x30);

        
        current_element = possition_element_vertically(create_textbox_element(buffer, 3, &app.ui_elements), 5, last_element);
        // current_element = possition_element_horizontally(create_textbox_element(buffer, 3, &app.ui_elements), 5, last_element);
        current_element->id = i;

        current_element->nav.top = last_element;

        if (last_element)
            last_element->nav.bottom = current_element;

        if (first_element == NULL)
            first_element = current_element;

        last_element = current_element;
    }

    last_element->nav.bottom = first_element;
    first_element->nav.top = last_element;

    active_framebuffer(0);

    gui_application_set_navigation_selection(&app, first_element);
    app.navigation_enabled = true;

    gui_event* event;

    while ((event = gui_application_get_next_event(&app)))
    {
        event_handler(event);
    }

    free_application(&app);

    return 0;
}

void event_handler(gui_event* event)
{
    switch (event->event_type)
    {
    case GUI_EVENT_TYPE_NAV_FOCUS_CHANGED:
        on_nav_focus_changed(event);
        break;
    case GUI_EVENT_TYPE_NAV_SELECT:
        on_nav_select(event);
        break;
    // case GUI_EVENT_TYPE_KEY_UP:
        // printf("Key UP! %x\n", *((keypad_state*)event->event_data));
        // break;
    // case GUI_EVENT_TYPE_KEY_DOWN:
        // printf("Key DOWN! %x\n", *((keypad_state*)event->event_data));
        // break;
    default:
        break;
    }

    gui_application_defult_event_handler(&app, event);
}

void on_nav_focus_changed(gui_event* event)
{
    gui_event_nav_focus_changed* event_data = (gui_event_nav_focus_changed*)event->event_data;
    int idA = event_data->previous_selection ? event_data->previous_selection->id : -1;
    int idB = event_data->new_selection ? event_data->new_selection->id : -1;


    printf("Focus Changed from %d to %d\n", idA, idB);
}

void on_nav_select(gui_event* event)
{
    int id = event->event_data ? ((gui_element*)event->event_data)->id : -1;


    printf("Navigation select input: %d\n", id);
}