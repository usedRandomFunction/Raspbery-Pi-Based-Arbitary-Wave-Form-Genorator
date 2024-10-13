#include "io/filesystem.h"

#include "lib/timing.h"
#include "io/putchar.h"
#include "lib/memory.h"
#include "io/printf.h"
#include "io/mbr.h"
#include "io/sd.h"

#include <stdbool.h>

// Using the MBR it fins the first fat 32 partition
// @param entry A pointer to copy the partition info into
// @param found_multiple set to true if more then one fat 32 partition
// @return true if found a fat 32 partition
static bool s_find_parition_from_mbr(master_boot_record_partition_table_entry* entry, bool* found_multiple);

typedef struct {
    char            jmp[3];
    char            oem[8];
    unsigned char   bps0;
    unsigned char   bps1;
    unsigned char   spc;
    unsigned short  rsc;
    unsigned char   nf;
    unsigned char   nr0;
    unsigned char   nr1;
    unsigned short  ts16;
    unsigned char   media;
    unsigned short  spf16;
    unsigned short  spt;
    unsigned short  nh;
    unsigned int    hs;
    unsigned int    ts32;
    unsigned int    spf32;
    unsigned int    flg;
    unsigned int    rc;
    char            vol[6];
    char            fst[8];
    char            dmy[20];
    char            fst2[8];
} __attribute__((packed)) bpb_t;

fat32_fs* initialize_filesystem_from_media()
{
    printf("Initialize filesystem from media\n");

    master_boot_record_partition_table_entry partition;
    bool multiple_found = false;

    if (!s_find_parition_from_mbr(&partition, &multiple_found))
        return NULL;

    if (multiple_found)
    {
        printf("Initialization paused for ten seconds, to allow user to ack");

        for (int i = 0; i < 10; i++)
        {
            putchar('.');
            delay_milliseconds(1000);
        }
        putchar('\n');
    }

    bios_parameter_block* bpb = malloc(512);
    extened_boot_record_FAT32* ebr = (extened_boot_record_FAT32*)void_ptr_offset_bytes(bpb, sizeof(bios_parameter_block));

    if (sd_readblock(partition.lba_of_partition_start, bpb, 1) == 0)
    {
        free(bpb);
        return NULL;
    }

    if (bpb->bytes_per_sector != 512)
    {
        printf("Error: Bytes Per Sector != 512");
        free(bpb);
        return NULL;
    }

    fat32_fs* fs = malloc(sizeof(fat32_fs)); 

    fs->number_of_file_allocation_tables = bpb->number_of_file_allocation_tables;
    fs->number_of_sectors_per_cluster = bpb->number_of_sectors_per_cluster;
    fs->first_fat_sector = bpb->number_of_reserved_sectors + partition.lba_of_partition_start;
    fs->sectors_per_fat = ebr->sectors_per_file_allocation_table;
    fs->root_cluster = ebr->cluster_number_of_root_directory;
    fs->data_sector = bpb->number_of_reserved_sectors + bpb->number_of_file_allocation_tables * ebr->sectors_per_file_allocation_table + partition.lba_of_partition_start;

    free(bpb);

    return fs;
}

bool s_find_parition_from_mbr(master_boot_record_partition_table_entry* entry, bool* found_multiple)
{
    master_boot_record* mbr = malloc(512);
    if (sd_readblock(0, mbr, 1) == 0)
        return false;

    printf("searching for partition, disk id: 0x%x\n", mbr->disk_id);

    uint8_t entry_id = 255;

    for (int i = 0; i < 4; i++)
    {
        printf("    partition %d: %s\n", i, (mbr->partitions[i].partition_type == 0xC ? "FAT32 (LBA)" : "Unkown"));

        if (mbr->partitions[i].partition_type != 0xC)
            continue; // Only keep checking if it is FAT32 (LBA)

        if (entry_id == 255)
        {
            entry_id = i;
            continue;
        }

        *found_multiple = true;
    }

    if (entry_id == 255)
    {
        printf("Failed to find FAT32 partition!\n");
        free(mbr);
        return false;
    }

    memcpy(entry, mbr->partitions + entry_id, sizeof(*entry));
    free(mbr);
    
    if (*found_multiple)
    {
        printf("Warning: More then 1 FAT32 partition was found, defulting to lowest.\n");
    }
    printf("Selected partition %d\n", entry_id);

    return true;
}