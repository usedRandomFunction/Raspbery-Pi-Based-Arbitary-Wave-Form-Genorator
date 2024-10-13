#include "lib/user_program.h"
#include "lib/config_file.h"
#include "io/file_access.h"
#include "lib/memory.h"
#include "io/printf.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int main()
{
    user_program_info program;

    if (!load_user_program_from_disk(&program, "hworld.cfg"))
        return -1;

    switch_to_addess_space_to_program(&program);

    printf("User program exicuted and returned: %d\n", execute_user_program(&program));
    destroy_user_program(&program);

    return 0;
}
