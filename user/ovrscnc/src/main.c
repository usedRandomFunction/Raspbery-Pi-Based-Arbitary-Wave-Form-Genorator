#include "common/display.h"
#include "common/keypad.h"
#include "common/memory.h"
#include "common/string.h"
#include "common/config.h"

// Draws a rect from (x0, y0) to (x1, y1), but only filling the edges, inward for boarder_size px
// @param x0 The X coordinate of the top left corner
// @param y0 The Y coordinate of the top left corner
// @param x1 The X coordinate of the bottom right corner
// @param y1 The Y coordinate of the bottom right corner
// @param boarder_size How many pixels the outline should go in
// @param The color (including alpha) to write
// @param buffer The ID of the target frame buffer, if -1 is given the active frame buffer will be used
void draw_outline(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t boarder_size, display_color color, int bufffer);

// Gets a signal digiet from the give keypad state.
// If more then one of the [0, 9] buttons are pressed
// the highest value will be reutrned
// @param state The keypad state to get the digiet from
// @return [0, 9] for the button, or -1 if no number keys are pressed
int get_digiet_from_keypad_state(keypad_state state);

// Draws boarders on the edge of the screen to help
// the user see where the edges are.
void drawboarders();

// Draws the title of the app
void drawtitle();

// lets the user chose between editing top, bottom, left right
void select_option();

// Lets the user input values into input_buffers
void take_user_input();

// Coverets the given string to a uint32, igoring non numerical charectors
// @param str The string to convert
// @return The string as a number
uint32_t string_to_u32(const char* str);

// Takes user input and updates overscan values
void update_overscan();

#define INPUT_BUFFER_SIZE 10

const char* selection_names[4] = {"Top: ", "Bottom: ", "Left: ", "Right: "};
char input_buffers[4][INPUT_BUFFER_SIZE];
int selection;

int main()
{
    for (int i = 0; i < 4; i++)
    {
        memset(input_buffers[i] + 1, INPUT_BUFFER_SIZE - 2, ' ');
        input_buffers[i][0] = '0';
        input_buffers[i][INPUT_BUFFER_SIZE - 1] = '\0';
    }
    selection = -1;

    active_framebuffer(0);
    drawboarders();
    drawtitle();
    
    
    

    while(1)
    {
        select_option(); // I.e top. bottom, left, right
        take_user_input();
        update_overscan();
    }

    return 0;
}



void drawboarders()
{
    uint32_t height = get_display_height() - 1;
    uint32_t width = get_display_width() - 1;

    draw_outline(0, 0, width, height, 1, FRAMEBUFFER_RGB(255, 0, 0), 0);
    draw_outline(1, 1, width - 1, height - 1, 1, FRAMEBUFFER_RGB(0, 255, 0), 0);
    draw_outline(2, 2, width - 2, height - 2, 10, FRAMEBUFFER_RGB(0, 0, 255), 0);
    draw_outline(12, 12, width - 12, height - 12, 40, FRAMEBUFFER_RGB(255, 0, 255), 0);
}

void drawtitle()
{
    uint32_t x = 100;
    uint32_t y = 100;

    DISPLAY_DRAW_STR_SIMPLE("Overscan config", &x, &y, 0);
}

void select_option()
{
    uint32_t start_x = 100;
    uint32_t start_y = 100;
    uint32_t x = start_x;
    uint32_t y = start_y;
    
    DISPLAY_DRAW_STR_SIMPLE("\n"
    "Select setting\n"
    "8: Top\n"
    "2: Bottom\n"
    "4: Left\n"
    "6: Right\n", &x, &y, 0);

    keypad_state last_state = get_keypad_state();
    while (selection == -1)
    {
        keypad_state current_state = get_keypad_state();
        keypad_state keydown = (current_state ^ last_state) & current_state;
        last_state = current_state;

        if (!keydown)       // If no key press skip
            continue;

        switch (get_digiet_from_keypad_state(keydown))
        {
        case 8:
            selection = 0;
            break;
        case 2:
            selection = 1;
            break;
        case 4:
            selection = 2;
            break;
        case 6:
            selection = 3;
            break;
        default:
            break;
        }
    }

    display_fill_rect(start_x, start_y, 500, y, FRAMEBUFFER_RGB(0, 0, 0), 0);
}

void take_user_input()
{
    char* input_buffer = input_buffers[selection];
    int i = 0;

    for ( ; i < INPUT_BUFFER_SIZE; i++)
        if (input_buffer[i] == ' ')
            break;

    uint32_t start_x = 200;
    uint32_t start_y = 200;
    uint32_t x = start_x;
    uint32_t y = start_y;
    DISPLAY_DRAW_STR_SIMPLE(selection_names[selection], &x, &y, 0);
    uint32_t value_start_x = x;
    DISPLAY_DRAW_STR_SIMPLE(input_buffer, &x, &y, 0);

    keypad_state last_state = get_keypad_state();
    while (1)
    {
        keypad_state current_state = get_keypad_state();
        keypad_state keydown = (current_state ^ last_state) & current_state;
        last_state = current_state;

        if (!keydown)       // If no key press skip
            continue;

        if (last_state & KEYPAD_STATE_BUTTON_DEL && i > 0)
            input_buffer[--i] = ' ';
        
        if (last_state & KEYPAD_STATE_BUTTON_ENT)
            break;


        int digit = get_digiet_from_keypad_state(keydown);

        if (digit != -1 && i < INPUT_BUFFER_SIZE)
            input_buffer[i++] = (char)(0x30 + digit);
            
        x = value_start_x;
        y = 200;
        DISPLAY_DRAW_STR_SIMPLE(input_buffer, &x, &y, 0);
    }

    if (i == 0)
        input_buffer[0] = '0';


    selection = -1;
    display_fill_rect(start_x, start_y, x, y + 16, FRAMEBUFFER_RGB(0, 0, 0), 0);
}

void update_overscan()
{
    uint32_t top = string_to_u32(input_buffers[0]);
    uint32_t bottom = string_to_u32(input_buffers[1]);
    uint32_t left = string_to_u32(input_buffers[2]);
    uint32_t right = string_to_u32(input_buffers[3]);

    set_display_overscan(top, bottom, left, right);
}


uint32_t string_to_u32(const char* str)
{
    uint32_t val = 0;

    do
    {
        uint32_t x = (uint32_t)*str;
        x -= 0x30;

        if (x > 9)
            continue;

        val *= 10;
        val += x;
    }
    while (*(++str) != '\0');
    

    return val;
}


int get_digiet_from_keypad_state(keypad_state state)
{
    if (state & KEYPAD_STATE_BUTTON_9)
        return 9;
    else if (state & KEYPAD_STATE_BUTTON_8)
        return 8;
    else if (state & KEYPAD_STATE_BUTTON_7)
        return 7;
    else if (state & KEYPAD_STATE_BUTTON_6)
        return 6;
    else if (state & KEYPAD_STATE_BUTTON_5)
        return 5;
    else if (state & KEYPAD_STATE_BUTTON_4)
        return 4;
    else if (state & KEYPAD_STATE_BUTTON_3)
        return 3;
    else if (state & KEYPAD_STATE_BUTTON_2)
        return 2;
    else if (state & KEYPAD_STATE_BUTTON_1)
        return 1;
    else if (state & KEYPAD_STATE_BUTTON_0)
        return 0;

    return -1;
}

void draw_outline(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t boarder_size, display_color color, int bufffer)
{
    boarder_size--;

    display_fill_rect(x0, y0, x1, y0 + boarder_size, color, bufffer);   // Top
    display_fill_rect(x0, y1 - boarder_size, x1, y1, color, bufffer);   // Bottom

    display_fill_rect(x0, y0, x0 + boarder_size, y1, color, bufffer);   // Left
    display_fill_rect(x1 - boarder_size, y0, x1, y1, color, bufffer);   // right
}