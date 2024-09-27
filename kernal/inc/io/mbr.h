#ifndef MBR_H
#define MBR_H

#include <stdint.h>

struct master_boot_record_partition_table_entry
{
    uint8_t drive_attributes;
    uint8_t chs_address_of_partition_start[3];
    uint8_t  partition_type;
    uint8_t chs_address_of_partition_last_sector[3];
    uint32_t lba_of_partition_start;
    uint32_t number_of_sectors_in_partition;
}__attribute__((packed));

typedef struct master_boot_record_partition_table_entry master_boot_record_partition_table_entry;

struct master_boot_record
{
    uint8_t bootstrap_code[440];
    uint32_t disk_id;
    uint16_t reserved;
    struct master_boot_record_partition_table_entry partitions[4];
}__attribute__((packed));

typedef struct master_boot_record master_boot_record;



#endif