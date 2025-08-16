#include "common/program_managment.h"
#include "common/basic_io.h"
#include "common/file_io.h"
#include "common/string.h"
#include "common/memory.h"

#include "gui/elements/standard.h"
#include "gui/application.h"

#ifndef MAX_PROGRAMS
#define MAX_PROGRAMS 128
#endif

char program_names[MAX_PROGRAMS][32]; // Buffer to save the names of apps in
int number_of_programs_found;
char target_application_path[256];  // Stores the path to the applicaiton


uint32_t scroll_window_max_height;
uint32_t scroll_window_min_height;

gui_application app;

gui_element* file_path_indicator;
gui_element* scroll_container;

// Creates the GUI for the application and all buttons
void create_gui();

// Prints the name of the given slection
// @param id the index of the program in the program_names array
void print_new_selection(int id);

// Checks if the scroll_container needs to be moved, based of the selected element
// @param new_selection gui_event_nav_focus_changed::new_selection
// @return True if scrolling is required, false if not
bool is_scrolling_required(gui_element* new_selection);

// Moves the scroll container, based of the selected element
// @param new_selection gui_event_nav_focus_changed::new_selection
void handle_scrolling(gui_element* new_selection);

// handler for nav_focuse_changed event
// @param event The event to handle
void on_nav_focus_changed(gui_event* event);

// handler for nav_select event
// @param event The event to handle
void on_nav_select(gui_event* event);

// Handles events for the app
// @param event Pointer to current event
void event_handler(gui_event* event);

int main() 
{
    int dir = diropen("/user/apps/");

    if (dir == -1)
        return -1;

    printf("Shearching /user/apps/ for applicaitons\n");

    dirrectory_entry dir_entry;
    number_of_programs_found = 0;
    
    while (number_of_programs_found < MAX_PROGRAMS && dirread(dir, &dir_entry) > 0)
    {
        if (strcmp(dir_entry.extention, "CFG") != 0)
            continue;       // Skip all entrys that arn't apps

        if (strcmp(dir_entry.name, "MAINMENU") == 0)
            continue;       // Dont include the main menu in the list of apps

        strcpy_s(dir_entry.name, 32, program_names[number_of_programs_found++]);
        printf("%s\n", dir_entry.name);
    }
    
    dirclose(dir);

    create_gui();

    active_framebuffer(0);

    gui_event* event;

    while ((event = gui_application_get_next_event(&app)))
    {
        event_handler(event);
    }
    
    return 0;
}

void create_gui()
{
    initialize_gui_application(&app);

    uint32_t height = get_display_height();
    uint32_t width = get_display_width();

    // Create Background
    gui_element* background = create_frame_element(&app.ui_elements);
    background->size.y = height;
    background->size.x = width;
    background->flags |= GUI_ELEMENT_FLAGS_DISABLED;

    scroll_container = create_element(&app.ui_elements);

    // Since we want centered text its easier to have the background be centered
    gui_element* page_header_background = create_frame_element(&app.ui_elements);
    gui_element* page_header = create_text_element("Select application", &app.ui_elements);
    page_header->position.x = width / 2;
    size_textbox_element(page_header, 3);   // We need to set size.y so we run possition_element_vertically
    page_header->size.y *= 2;               // Max it bigger for astetics
    page_header_background->size.y = page_header->size.y;
    page_header_background->size.x = width;
    page_header_background->flags |= GUI_ELEMENT_FLAGS_DISABLED;

    gui_element* bottom_header = create_frame_element(&app.ui_elements);
    bottom_header->position.y = height - page_header->size.y;
    bottom_header->size.y = page_header->size.y;
    bottom_header->size.x = width;
    bottom_header->flags |= GUI_ELEMENT_FLAGS_DISABLED;
    scroll_window_min_height = bottom_header->position.y + 1;

    // Create the spot to show the file path of the app that is selected
    strcpy_s("No selection", 256, target_application_path);
    file_path_indicator = create_text_element(target_application_path, &app.ui_elements);
    size_textbox_element(file_path_indicator, 3);       // We need size.y to possition it veritcally
    file_path_indicator->position.y = height - file_path_indicator->size.y;
    file_path_indicator->position.x = width - 3;
    right_allign_text_element(file_path_indicator);
    file_path_indicator->size.x = 0;
    file_path_indicator->color_disabled.forground = bottom_header->color_disabled.background;   // We cant get the color from the text
                                                                                                // as it is transparent by default
    gui_element* first_element = NULL;
    gui_element* last_element = page_header; // By putting the header here we can use possition element functions for everything

    for (int i = 0; i < number_of_programs_found; i++)
    {
        gui_element* current_element = create_textbox_element(program_names[i], 3, &scroll_container->sub_elements);
        possition_element_vertically(current_element, 5, last_element);
        current_element->parent = scroll_container;
        current_element->id = i;

        center_text_element(last_element, true, false);
        current_element->nav.top = last_element;
        last_element->nav.bottom = current_element;

        if (first_element == NULL)
            first_element = current_element;

        last_element = current_element;
    }

    first_element->nav.top = last_element;
    last_element->nav.bottom = first_element;
    center_text_element(last_element, true, false);
    scroll_window_max_height = first_element->position.y;

    gui_application_set_navigation_selection(&app, first_element);
    app.navigation_enabled = true;
}

void print_new_selection(int id)
{   
    putchar('\r');

    for (int i = 0; i < 42; i++)
        putchar(' ');

    printf("\rSelected: %s", program_names[id]);
}

bool is_scrolling_required(gui_element* new_selection)
{
    uint32_t global_y_possiton = new_selection->position.y + scroll_container->position.y;

    if (global_y_possiton < scroll_window_max_height)   // Y is 0 on the top so maxium y value is maxium height
        return true;

    if ((global_y_possiton + new_selection->size.y) > scroll_window_min_height)
        return true;

    return false;
}

// Moves the scroll container, based of the selected element
// @param new_selection gui_event_nav_focus_changed::new_selection
void handle_scrolling(gui_element* new_selection)
{
    uint32_t new_scroll_possiotn = scroll_window_max_height - new_selection->position.y;
    scroll_container->position.y = new_scroll_possiotn;
    redraw_gui_application(&app);
}

void on_nav_focus_changed(gui_event* event)
{
    gui_event_nav_focus_changed* data = (gui_event_nav_focus_changed*)event->event_data;
    
    if (!data || !data->new_selection)
        return;

    bool scrolling_is_required = is_scrolling_required(data->new_selection);


    if (!scrolling_is_required)
    {
        // First we blank the current text
        file_path_indicator->flags |= GUI_ELEMENT_FLAGS_DISABLED;   // Color is set up using the dissabled color, so we just toggle
        draw_element(file_path_indicator, app.target_buffer);
    }

    int id = data->new_selection->id;

    memclr(target_application_path, 256);
    strcpy_s("/user/apps/", 256, target_application_path);
    strcat_s(target_application_path, 256, program_names[id]);
    strcat_s(target_application_path, 256, ".cfg");

    print_new_selection(id);

    file_path_indicator->position.x = get_display_width() - 3;
    right_allign_text_element(file_path_indicator);

    // Now redraw
    if (!scrolling_is_required)
    {
        file_path_indicator->flags &= ~GUI_ELEMENT_FLAGS_DISABLED;
        draw_element(file_path_indicator, app.target_buffer);
    }
    else
        handle_scrolling(data->new_selection);
}

void on_nav_select(gui_event* event)
{
    gui_element* target = (gui_element*)event->event_data;

    if (!target)
        return;
    
    printf("Switching to %s\n", target_application_path);
    switch_to(target_application_path);
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
    default:
        break;
    }

    gui_application_defult_event_handler(&app, event);
}
