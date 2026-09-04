#include "simple_waveform_input/input.h"

#include "gui/application.h"
#include "common/basic_io.h"
#include "common/display.h"
#include <stdbool.h>

gui_application app;

// Handles events for the app
// @param event Pointer to current event
void event_handler(gui_event* event);

#include "dynamic_array/dynamic_array.h"

int main()
{
    initialize_gui_application(&app);
   
    simple_waveform_input input;
    gui_element* test = &input.continer;

    initalize_simple_waveform_input(&input, 500, 500);
    insert_dynamic_array(&test, app.ui_elements.number_of_entrys, &app.ui_elements);

    active_framebuffer(0);

    gui_application_set_navigation_selection(&app, input.top);
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

