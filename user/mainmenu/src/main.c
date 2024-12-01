#include "common/program_managment.h"
#include "common/basic_io.h"
#include "common/file_io.h"
#include "common/keypad.h"
#include "common/string.h"

#define MAX_PROGRAMS 128

char program_names[MAX_PROGRAMS][32]; // Buffer to save the names of apps in
int number_of_programs_found;

void main_app_loop();

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

    main_app_loop();
    
    return 0;
}

void print_new_selection(int id)
{   
    putchar('\r');

    for (int i = 0; i < 42; i++)
        putchar(' ');

    printf("\rSelected: %s", program_names[id]);
}

void launch_selection(int id)
{
    char path_buffer[256];
    strcpy_s("/user/apps/", 256, path_buffer);
    strcat_s(path_buffer, 256, program_names[id]);
    strcat_s(path_buffer, 256, ".cfg");

    printf("\nSwitching to %s\n", path_buffer);
    switch_to(path_buffer);
}

void main_app_loop()
{
    keypad_state previous_keypad_state = 0;
    int current_selection = 0;

    putchar('\n');
    print_new_selection(0);

    while (1)
    {
        keypad_state current_keypad_state = get_keypad_state();

        // This mess finds out what keys have just started to be pressed
        keypad_state key_down = (current_keypad_state ^ previous_keypad_state) & current_keypad_state;
        
        if (key_down == 0)  // TODO maby a delay here
            continue;       // Dont do anything if no keys are pressed

        if (key_down & KEYPAD_STATE_BUTTON_8) // Scroll up
        {
            current_selection--;

            if (current_selection < 0)  // Loop back if we scroll to far
                current_selection = number_of_programs_found - 1;
        }

        if (key_down & KEYPAD_STATE_BUTTON_2) // Scroll down
        {
            current_selection++;

            if (current_selection >= number_of_programs_found)
                current_selection = 0;
        }

        if (key_down & KEYPAD_STATE_BUTTON_ENT) // Launch
            launch_selection(current_selection);
        

        print_new_selection(current_selection);
    }
}