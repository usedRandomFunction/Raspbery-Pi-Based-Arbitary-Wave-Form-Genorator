// #include "run_time_kernal_config.h"
// #include "lib/user_program.h"
// #include "lib/config_file.h"
// #include "io/file_access.h"
#include "lib/memory.h"
// #include "lib/timing.h"
#include "io/printf.h"
#include "io/sd.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


int main()
{
    uint8_t buf[512];

    for (int i = 0; i < 512; i++)
        buf[i] = i % 256;

    sd_writeblock(123456, buf, 1);

    printf("write done");

    sd_readblock(123456, buf, 1);

    for (int i = 0; i < 512; i++)
    {
        if (buf[i] != i % 256)
        {
            printf("discepency at %d\n", i);
            return -1;
        }
    }

    return 0;
    // while (1)
    // {

    //     user_program_info program;
    //      if (!load_user_program_from_disk(&program, main_interface_app_path))
    //         return -1;


    //     printf("Running main interface program\n");

    //     switch_to_addess_space_to_program(&program);
    //     uint64_t porgam_start_count = get_timer_count();

    //     int return_value = execute_user_program(&program);

    //     uint64_t porgam_end_count = get_timer_count();

    //     printf("Main interface program returned %d\n", return_value);
    //     destroy_user_program(&program);

    //     if ((porgam_end_count - porgam_start_count) < (get_timer_freqency() / 4)) // If program returned in less then 0.25 sec abort
    //     {
    //         printf("Main interface program took less then 0.25 seconds the reutrn, \nassuming error occured and aborting\n");
    //         return -2;
    //     }
    // }
}