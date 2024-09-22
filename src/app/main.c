#include "io/file_access.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "io/printf.h"
#include "io/sd.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int main()
{
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    // PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

    int file = open("test.txt", 0);

    printf("FD = %d\n", file);
    if (file == -1)
        return -1;

    // lseek(file, -10, SEEK_END);

    char buf[1024];

    printf("%d\n%s\n", read(file, buf, SIZE_MAX), buf);

    close(file);

    return 0;
}
