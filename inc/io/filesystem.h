#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

struct bios_parameter_block
{
    uint8_t  jmp[3]; // First 3 bytes of boot sector are for jump instruction
    uint8_t  oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t  number_of_sectors_per_cluster;
    uint16_t number_of_reserved_sectors;
    uint8_t  number_of_file_allocation_tables;
    uint16_t number_of_root_directory_entries;
    uint16_t total_number_of_sectors;
    uint8_t  media_descriptor_types;
    uint16_t number_of_sectors_file_allocation_table;
    uint16_t number_of_sectors_per_track;
    uint16_t number_of_sectors_heads_of_media;
    uint32_t number_of_hidden_sectors;
    uint32_t large_sector_count;
}__attribute__((packed));

typedef struct bios_parameter_block bios_parameter_block;

struct extened_boot_record_FAT32 // Offset by 0x24 from the start of the boot record
{
    uint32_t sectors_per_file_allocation_table;
    uint16_t flags;
    uint16_t version_number;
    uint32_t cluster_number_of_root_directory;
    uint16_t sector_number_of_fs_info;
    uint16_t sector_number_of_backup_boot_sector;
    uint8_t  reserved_1[12];
    uint8_t  drive_number;
    uint8_t  reserved_2[1];
    uint8_t  signature;
    uint32_t volume_serial_number;
    char     volume_lable[11];
    char     system_identifier_string[8];
}__attribute__((packed));

typedef struct extened_boot_record_FAT32 extened_boot_record_FAT32;

struct fs_info
{
    uint32_t lead_siqnature; // Must be 0x41615252
    uint8_t  reserved_1[480];
    uint32_t second_signature; // Must be 0x61417272
    uint32_t free_cluster_count;
    uint32_t clust_number_begin_shearching_at;
    uint8_t  reserved_2[12];
    uint32_t trail_signaute; // Must be 0xAA550000
}__attribute__((packed));

typedef struct fs_info fs_info;

struct fat_directory_entry
{
    char     file_name[11];
    uint8_t  attributes;
    uint8_t  reserved;
    uint8_t  creation_time_hudriths;
    uint16_t creation_time_hms;
    uint16_t creation_time_date;
    uint16_t last_access_date;
    uint16_t first_cluster_number_higher_16_bits;
    uint16_t modification_time_hms;
    uint16_t modification_time_date;
    uint16_t first_cluster_number_lowwer_16_bits;
    uint32_t file_size_bytes;
}__attribute__((packed));

typedef struct fat_directory_entry fat_directory_entry;

enum
{
    FAT_DIRECTORY_ATTRIBUTES_READ_ONLY  = 0x01,
    FAT_DIRECTORY_ATTRIBUTES_HIDDEN     = 0x02,
    FAT_DIRECTORY_ATTRIBUTES_SYSTEM     = 0x04,
    FAT_DIRECTORY_ATTRIBUTES_VOLUME_ID  = 0x08,
    FAT_DIRECTORY_ATTRIBUTES_DIRECTORY  = 0x10,
    FAT_DIRECTORY_ATTRIBUTES_ARCHIVE    = 0x20,
    FAT_DIRECTORY_ATTRIBUTES_LFN        = 0x0F
};

struct fat32_fs // Used by the kernel to store info about a fat 32 file system
{
    uint8_t  number_of_sectors_per_cluster;
    uint32_t first_fat_sector;
    uint32_t root_cluster;
    uint32_t root_sector;
    
};

typedef struct fat32_fs fat32_fs;

// initialize the filesystem using the SD card
// @return Pointer to fat32_fs struct or NULL
fat32_fs* initialize_filesystem_from_media();

#endif