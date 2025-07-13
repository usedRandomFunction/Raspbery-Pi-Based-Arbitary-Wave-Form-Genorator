#include "gui/application.h"
#include "gui/elements.h"
#include "gui/elements/complex.h"

#include "common/basic_io.h"
#include "common/display.h"


gui_application app;

// Handles events for the app
// @param event Pointer to current event
void event_handler(gui_event* event);


int main()
{
    initialize_gui_application(&app);

    gui_element* current_element = create_frame_element(&app.ui_elements);
    current_element->size.x = get_display_width() - 1;
    current_element->size.y = get_display_height() - 1;
    current_element->flags |= GUI_ELEMENT_FLAGS_DISABLED;

    current_element = create_integer_input_element(0, -10, 10, 2, 3, &app.ui_elements);
    current_element->position.y = 100;
    current_element->position.x = 100;

    active_framebuffer(0);

    gui_application_set_navigation_selection(&app, current_element);
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
    default:
        break;
    }

    gui_application_defult_event_handler(&app, event);
}

