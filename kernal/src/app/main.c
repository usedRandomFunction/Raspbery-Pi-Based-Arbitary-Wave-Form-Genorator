#include "run_time_kernal_config.h"
#include "lib/user_program.h"
#include "lib/timing.h"
#include "lib/memory.h"
#include "io/printf.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int main()
{
    while (1)
    {
        const char* next_app = use_program_requested_switch == NULL ?
            main_interface_app_path : use_program_requested_switch;

        user_program_info program;
        bool success = load_user_program_from_disk(&program, next_app);

        if (use_program_requested_switch != NULL)
        {
            free(use_program_requested_switch);
            use_program_requested_switch = NULL;
        }

        if (!success)
            return -1;

        printf("Running program\n");

        switch_to_addess_space_to_program(&program);
        uint64_t porgam_start_count = get_timer_count();

        int return_value = execute_user_program(&program);

        uint64_t porgam_end_count = get_timer_count();

        printf("program returned %d\n", return_value);
        destroy_user_program(&program);

        if ((porgam_end_count - porgam_start_count) < (get_timer_frequency() / 4)   // If program returned in less then 0.25 sec abort
        && (next_app == main_interface_app_path))                                   // But only if it is the main interface app
        {
            printf("Main interface program took less then 0.25 seconds to reutrn, \nassuming error occured and aborting\n");
            return -2;
        }
    }
}