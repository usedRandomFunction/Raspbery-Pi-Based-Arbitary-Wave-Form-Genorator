#include "io/filesystem.h"
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

    fat32_fs* fs = initialize_filesystem_from_media();

    if (fs == NULL)
    {
        printf("Failed to initialize file system!\n");
        return -1;
    }

    fat_directory_entry* ent = malloc(512);
    if (sd_readblock(fs->root_sector, (uint8_t*)ent, 1) == 0)
        return -1;

    char name[12];
    memclr(name, 12);

    printf("attempting to read root dirrectory!\n");

    for (int i = 0; ent[i].file_name[0] != '\0' && i < (512 / sizeof(*ent)); i++)
    {
        if (ent[i].file_name[0] == (char)0xE5)
            continue;

        memcpy(name, ent[i].file_name, 11);
        printf("%s: %d bytes (%s)\n", name, ent[i].file_size_bytes, (ent[i].attributes & FAT_DIRECTORY_ATTRIBUTES_DIRECTORY ? "dirrectory" : "file"));
    }

    free(ent);
    free(fs);

    return 0;
}
