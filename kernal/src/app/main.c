#include "run_time_kernal_config.h"
#include "lib/user_program.h"
#include "lib/config_file.h"
#include "lib/timing.h"
#include "io/printf.h"
#include "io/keypad.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


int main()
{
    keypad_init();

    keypad_state old_state = 0;

    while (1)
    {
        keypad_state new_state = get_keypad_state();

        if ((new_state ^ old_state) == 0)
            continue;

        printf("New keypad state: %x\n", new_state);

        old_state = new_state;
        // user_program_info program;
        //  if (!load_user_program_from_disk(&program, main_interface_app_path))
        //     return -1;


        // printf("Running main interface program\n");

        // switch_to_addess_space_to_program(&program);
        // uint64_t porgam_start_count = get_timer_count();

        // int return_value = execute_user_program(&program);

        // uint64_t porgam_end_count = get_timer_count();

        // printf("Main interface program returned %d\n", return_value);
        // destroy_user_program(&program);

        // if ((porgam_end_count - porgam_start_count) < (get_timer_frequency() / 4)) // If program returned in less then 0.25 sec abort
        // {
        //     printf("Main interface program took less then 0.25 seconds to reutrn, \nassuming error occured and aborting\n");
        //     return -2;
        // }
    }
}