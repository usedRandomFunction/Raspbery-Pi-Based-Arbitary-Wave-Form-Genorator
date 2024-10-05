// #include "lib/translation_table.h"
#include "lib/user_program.h"
#include "io/file_access.h"
// #include "lib/memory.h"
// #include "lib/alloc.h"
#include "io/printf.h"
// #include "lib/mmu.h"
#include "io/sd.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// extern void test_el0(void* user_memory);

int main()
{
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    user_program_info program;

    int file = open("hworld.img", 0);

    if (file == -1)
    {
        printf("Failed to load user program\n");
        return -1;
    }

    size_t progrma_size = get_file_size(file);

    if (progrma_size == -1)
        return -1;

    if (initialize_monolithic_user_program(&program, (void*)0x000040000000, progrma_size, (void*)0x000040000000, 1024) == false)
    {
        close(file);
        return -1;
    }

    switch_to_addess_space_to_program(&program);

    size_t bytes_read = read(file, (void*)0x000040000000, progrma_size);

    close(file);
    
    if (bytes_read == -1)
    {
        printf("Failed to load user program");
        destroy_user_program(&program);
        return - 1;
    }

    printf("User program exicuted and returned: %d\n", execute_user_program(&program));
    destroy_user_program(&program);

    return 0;
}
