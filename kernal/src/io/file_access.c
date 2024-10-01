#include "io/file_access.h"

#include "lib/dynamic_array.h"
#include "io/filesystem.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/math.h"
#include "io/printf.h"
#include "io/sd.h"

#include <stdint.h>

struct file_discriptor_metadata
{
    uint32_t first_cluster_number;
    uint32_t current_cluster_number;
    uint32_t current_offset;
    size_t file_size_bytes;
};

typedef struct file_discriptor_metadata file_discriptor_metadata;

struct fd_hash_table_entry
{
    file_discriptor_metadata metadata;
    int hash;
};

typedef struct fd_hash_table_entry fd_hash_table_entry;

static dynamic_array s_fd_hash_table;
extern fat32_fs* root_file_system;

static bool s_less_then_fd_table_entry(void* A, void* B);
static bool s_equal_to_fd_table_entry(void* A, void* B);

// Shearches the root dirrectory for a file
// @param path Null terminated string containing path
// @return pointer to directory_entry structer or NULL if file does not exist
// @note path Does not start with '/' so "/a.txt" turns into "a.txt" (local paths are not supported)
static fat_directory_entry* s_find_file_from_path(const char* path);

// Used by s_find_file_from_path to find a file
// @param path Path to file form current dirrectory
// @param current_dirrectory_cluster_number Cluster number of current dirrectory
// @param fat_buffer A 512 byte buffer to store a sector of the file allocation table into
// @param dirrectory_cluster A 512 * sectors_per_cluster buffer to store the dirrectory into
// @return pointer to directory_entry structer or NULL if file does not exist
static fat_directory_entry* s_find_file_recursive(const char* path, uint32_t current_dirrectory_cluster_number, uint32_t* fat_buffer, fat_directory_entry* dirrectory_cluster);

// Used by s_find_file_recursive to get the correctly formated file name, also detects if we still are in a dirrectory
// @param path Path of the file
// @param buffer A 11 byte buffer to write into
// @return -1 if failed int32_max if file or the number of bytes taken (including traling /) if dirrectory
static int32_t s_format_file_name_8_3_standered(const char* path, char* buffer);

void initialize_file_access()
{
    initialize_dynamic_array(sizeof(fd_hash_table_entry), 0, &s_fd_hash_table);
}

int open(const char* path, int flags)
{
    if (flags != 0)
    {
        printf("Erorr: Failed to open file \"%s\", only read mode is supported!");
    }

    if(path[0] == '/' || path[0] == '\\') 
        path++; // Just used so /a.txt becomes a.txt 

    int file_discriptor = (int)djb2_hash_uppercase(path);
    
    if (file_discriptor < 0)
        file_discriptor = -file_discriptor;

    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = file_discriptor;
 
    bool allready_opened = false;

    int index = (int)dynamic_array_find_closest_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, &allready_opened,
        s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (allready_opened == true)
        return -1;

    fat_directory_entry* file = s_find_file_from_path(path);

    if (file == NULL)
        return -1;

    fd_hash_table_entry entry;
    entry.metadata.first_cluster_number = (file->first_cluster_number_higher_16_bits << 16) | (file->first_cluster_number_lowwer_16_bits);
    entry.metadata.file_size_bytes = file->file_size_bytes;
    entry.metadata.current_cluster_number = entry.metadata.first_cluster_number;
    entry.metadata.current_offset = 0;
    entry.hash = file_discriptor;
    free(file);

    if (insert_dynamic_array(&entry, (size_t)index, &s_fd_hash_table) == false)
        return -1; // Failed to insert

    return file_discriptor;
}

int32_t s_format_file_name_8_3_standered(const char* path, char* name_buffer)
{
    size_t size_of_current_entry_name = 1;
    bool is_filling_extention = false;
    bool is_looking_for_file = true;
    int extention_offset = 3;

    memset(name_buffer, 11, (uint8_t)' ');

    for (int i = 0 ; i < 12 && path[i] != '\0'; i++, size_of_current_entry_name++)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            is_looking_for_file = false;
            break;
        }

        if (path[i] == '.')
        {
            is_filling_extention = true;
            continue;
        }

        if ((is_filling_extention == false && i >= 8) || (extention_offset == 0 && path[i + 1] != '\0'))
        {
            printf("Erorr: Long file name is not supported!\n");
            return -1;
        }
        
        if (is_filling_extention == false)
        {
            name_buffer[i] = toupper(path[i]);
            continue;
        }
        
        name_buffer[11 - extention_offset--] = toupper(path[i]);
    }

    if (is_looking_for_file && is_filling_extention)
        return INT32_MAX;

    if (is_looking_for_file == false)
        return size_of_current_entry_name;

    return -1;
}

fat_directory_entry* s_find_file_recursive(const char* path, uint32_t current_dirrectory_cluster_number, uint32_t* fat_buffer, fat_directory_entry* dirrectory_cluster)
{
    // char name_buffer[12]; // dont delete these debug lines (even if commented out), bc then you will need them
    char name_buffer[11];
    int32_t name_offset = s_format_file_name_8_3_standered(path, name_buffer);
    // name_buffer[11] = 0;
    // printf("Name To Check: %s\n", name_buffer);
    
    if (name_offset == -1) // Failed / invalid name / Is LFN
        return NULL;

    uint32_t next_cluster = current_dirrectory_cluster_number;
    fat_directory_entry* matching_dirrectory = NULL;
    uint32_t last_fat_sector = UINT32_MAX;

    while (next_cluster < 0x0FFFFFF8)
    {   
        uint32_t cluster_lba = root_file_system->data_sector + (next_cluster - 2) * root_file_system->number_of_sectors_per_cluster;
        
        if (sd_readblock(cluster_lba, dirrectory_cluster, 
            root_file_system->number_of_sectors_per_cluster) 
            != 512 * root_file_system->number_of_sectors_per_cluster)
        {
            printf("Erorr: Failed to read dirrecotry!\n");
            break;
        }

        uint32_t fat_sector = root_file_system->first_fat_sector + (next_cluster / (512 / 4));
        uint32_t fat_offset = (next_cluster % (512 / 4));

        if (last_fat_sector != fat_sector)
        {
            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");
                return NULL;
            }
            last_fat_sector = fat_sector;
        }

        next_cluster = fat_buffer[fat_offset] & 0x0FFFFFFF;

        for (int i = 0 ; i < ((512 * root_file_system->number_of_sectors_per_cluster) / sizeof(fat_directory_entry)); i++)
        {
            // Dont delete these debug lines (even if commented out), bc then you will need them
            // char buf[12];
            // buf[11] = '\0';
            // memcpy(buf, dirrectory_cluster[i].file_name, 11);

            // if (buf[0] != 0xE5 && buf[0] != '\0')
            // {
            //     printf("%s:%x\n", buf, (size_t)buf[0]);
            // }

            if (memcmp(name_buffer, dirrectory_cluster[i].file_name, 11) != 0)
                continue;

            matching_dirrectory = dirrectory_cluster + i;

            next_cluster = 0x0FFFFFFF; // Force the while loop to stop
            break;
        }
    }

    if (matching_dirrectory == NULL || (matching_dirrectory->attributes == FAT_DIRECTORY_ATTRIBUTES_LFN))
        return NULL;

    if (name_offset == INT32_MAX) // Is File
    {
        if (matching_dirrectory->attributes & FAT_DIRECTORY_ATTRIBUTES_DIRECTORY)
        {
            printf("Erorr: Expected file, found dirrectory!\n");
            return NULL;
        }

        fat_directory_entry* entry = malloc(sizeof(fat_directory_entry));
        memcpy(entry, matching_dirrectory, sizeof(fat_directory_entry));
        return entry;
    }

    if (matching_dirrectory->attributes & FAT_DIRECTORY_ATTRIBUTES_DIRECTORY)
    {
        printf("Erorr: Expected dirrectory, found somethign else!\n");

        return NULL;
    }

    uint32_t next_dirrectory_cluster = (matching_dirrectory->first_cluster_number_higher_16_bits << 16) | 
        (matching_dirrectory->first_cluster_number_lowwer_16_bits);

    return s_find_file_recursive(path + name_offset, next_dirrectory_cluster, fat_buffer, dirrectory_cluster);
}

fat_directory_entry* s_find_file_from_path(const char* path)
{
    fat_directory_entry* dirrectory_cluster = malloc(512 * root_file_system->number_of_sectors_per_cluster);
    uint32_t* fat_buffer = malloc(512);

    fat_directory_entry* entry = s_find_file_recursive(path, root_file_system->root_cluster, fat_buffer, dirrectory_cluster);

    free(dirrectory_cluster);
    free(fat_buffer);

    return entry;
}

size_t read(int fd, void* buf, size_t n)
{
    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = fd;

    size_t index = dynamic_array_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (index == -1)
        return -1;

    fd_hash_table_entry* file = (fd_hash_table_entry*)s_fd_hash_table.ptr;
    file += index;

    const uint32_t clsuter_size = root_file_system->number_of_sectors_per_cluster * 512;

    n = min(file->metadata.file_size_bytes - file->metadata.current_offset, n); // Make sure we dont try to read bytes that dont exist
    int64_t right_space_in_first_cluster = clsuter_size - (file->metadata.current_offset % clsuter_size);
    uint32_t number_of_bytes_to_read_from_first_clsuter = min(right_space_in_first_cluster, n);
    uint32_t number_of_bytes_to_read_from_last_clsuter = 0;
    uint32_t number_of_middle_clusters = 0;

    if (right_space_in_first_cluster < n)
    {
        uint32_t left_over_bytes = n - right_space_in_first_cluster;
        number_of_bytes_to_read_from_last_clsuter = left_over_bytes % clsuter_size;
        number_of_middle_clusters = left_over_bytes / clsuter_size;
    }

    uint8_t* file_temporay_buffer = NULL; // Used to copy sectors to when we're not copying the hole thing
    uint32_t cluster_lba = 0;
    uint8_t* buffer = (uint8_t*)buf;

    if (number_of_bytes_to_read_from_first_clsuter != 0 || number_of_bytes_to_read_from_last_clsuter != 0)
        file_temporay_buffer = malloc(clsuter_size);
    
    if (number_of_bytes_to_read_from_first_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_readblock(cluster_lba, file_temporay_buffer, root_file_system->number_of_sectors_per_cluster) != clsuter_size)
        {
            printf("Failed to read SD!\n");
            free(file_temporay_buffer);
            return -1;
        }

        memcpy(buffer, file_temporay_buffer + (file->metadata.current_offset % clsuter_size), number_of_bytes_to_read_from_first_clsuter);
        file->metadata.current_offset += number_of_bytes_to_read_from_first_clsuter;
        buffer += number_of_bytes_to_read_from_first_clsuter;
    }

    if (number_of_middle_clusters == 0 && number_of_bytes_to_read_from_last_clsuter == 0)
    {
        if (file_temporay_buffer != NULL)
            free(file_temporay_buffer);

        return (file->metadata.file_size_bytes == file->metadata.current_offset) ? 0 : n;
    }

    uint32_t last_fat_sector = UINT32_MAX;
    uint32_t* fat_buffer = malloc(512);
    uint32_t number_of_clusters_to_skip = number_of_middle_clusters + (number_of_bytes_to_read_from_last_clsuter == 0) ? 0 : 1;

    for ( ; number_of_clusters_to_skip > 0; number_of_clusters_to_skip--)
    {
        uint32_t fat_sector = root_file_system->first_fat_sector + (file->metadata.current_cluster_number / (512 / 4));
        uint32_t fat_offset = (file->metadata.current_cluster_number % (512 / 4));

        if (last_fat_sector != fat_sector)
        {
            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");
                if (file_temporay_buffer != NULL)
                    free(file_temporay_buffer);
                free (fat_buffer);

                return -1;
            }
            last_fat_sector = fat_sector;
        }

        uint32_t new_cluster_number = fat_buffer[fat_offset] & 0x0FFFFFFF;

        if (new_cluster_number >= 0x0FFFFFF8 && (number_of_middle_clusters != 0 || number_of_bytes_to_read_from_last_clsuter != 0))
        {
            printf("Erorr: Missmach between file size on dirrecotry entry and disk!\n");
            if (file_temporay_buffer != NULL)
                free(file_temporay_buffer);
            free (fat_buffer);

            return -1;
        }
        file->metadata.current_cluster_number = new_cluster_number;

        if (number_of_middle_clusters > 0)
        {
            cluster_lba = root_file_system->data_sector;
            cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

            if (sd_readblock(cluster_lba, buffer, root_file_system->number_of_sectors_per_cluster) != clsuter_size)
            {
                printf("Erorr: Failed to read SD!\n");
                if (file_temporay_buffer != NULL)
                    free(file_temporay_buffer);
                free (fat_buffer);

                return -1;
            }
            file->metadata.current_offset += clsuter_size;
            buffer += clsuter_size;
            
            number_of_middle_clusters--;
        }
    }

    free(fat_buffer);

    if (number_of_bytes_to_read_from_last_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_readblock(cluster_lba, file_temporay_buffer, root_file_system->number_of_sectors_per_cluster) != clsuter_size)
        {
            printf("Failed to read SD!\n");
            free(file_temporay_buffer);
            return -1;
        }

        memcpy(buffer, file_temporay_buffer, number_of_bytes_to_read_from_last_clsuter);
        file->metadata.current_offset += number_of_bytes_to_read_from_last_clsuter;
        buffer += number_of_bytes_to_read_from_last_clsuter;
    }


    if (file_temporay_buffer != NULL)
        free(file_temporay_buffer);

    file->metadata.current_offset += n;

    return (file->metadata.file_size_bytes == file->metadata.current_offset) ? 0 : n;
}

ptrdiff_t lseek(int fd, ptrdiff_t offset, int whence)
{
    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = fd;

    size_t index = dynamic_array_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (index == -1)
        return -1;

    fd_hash_table_entry* file = (fd_hash_table_entry*)s_fd_hash_table.ptr;
    file += index;
    ptrdiff_t new_offset = (ptrdiff_t)file->metadata.current_offset;

    switch (whence)
    {
    case SEEK_SET:
        new_offset = offset;
        break;
    case SEEK_CUR:
        new_offset += offset;
        break;
    case SEEK_END:
        new_offset = ((ptrdiff_t)file->metadata.file_size_bytes) + offset;
        break;
    default:
        return -1;
    }
    

    if (new_offset < 0 || new_offset > file->metadata.file_size_bytes)
        return -1;

    const uint32_t clsuter_size = root_file_system->number_of_sectors_per_cluster * 512;

    uint32_t old_cluster_offset = file->metadata.current_offset / clsuter_size;
    uint32_t new_cluster_offset = new_offset / clsuter_size;
    
    if (new_cluster_offset == old_cluster_offset)
    {
        file->metadata.current_offset = new_offset;
        return new_offset;
    }

    uint32_t number_of_clusters_skip = 0;
    uint32_t current_cluster_number = 0;

    if (new_cluster_offset > old_cluster_offset)
    {
        number_of_clusters_skip = new_cluster_offset - old_cluster_offset;
        current_cluster_number = file->metadata.current_cluster_number;
    }
    else
    {
        number_of_clusters_skip = new_cluster_offset;
        current_cluster_number = file->metadata.first_cluster_number;
    }


    uint32_t last_fat_sector = UINT32_MAX;
    uint32_t* fat_buffer = malloc(512);

    while (number_of_clusters_skip > 0)
    {
        uint32_t fat_sector = root_file_system->first_fat_sector + (current_cluster_number / (512 / 4));
        uint32_t fat_offset = (current_cluster_number % (512 / 4));

        if (last_fat_sector != fat_sector)
        {
            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");
                free (fat_buffer);

                return -1;
            }
            last_fat_sector = fat_sector;
        }

        current_cluster_number = fat_buffer[fat_offset] & 0x0FFFFFFF;

        if (current_cluster_number >= 0x0FFFFFF8)
        {
            printf("Erorr: Missmach between file size on dirrecotry entry and disk!\n");
            free (fat_buffer);

            return -1;
        }
        

        number_of_clusters_skip--;
    }

    free(fat_buffer);

    file->metadata.current_cluster_number = current_cluster_number;
    file->metadata.current_offset = new_offset;
    return new_offset;
}

size_t get_file_size(int fd)
{
    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = fd;

    size_t index = dynamic_array_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (index == -1)
        return -1;

    fd_hash_table_entry* entry = (fd_hash_table_entry*)s_fd_hash_table.ptr;
    entry += index;

    return entry->metadata.file_size_bytes;
}

int close(int fd)
{
    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = fd;

    size_t index = dynamic_array_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (index == -1)
        return -1;

    fd_hash_table_entry* entry = (fd_hash_table_entry*)s_fd_hash_table.ptr;
    entry += index;

    remove_dynamic_array_entry(index, &s_fd_hash_table);

    return 0;
}

bool s_less_then_fd_table_entry(void* A, void* B)
{
    fd_hash_table_entry* a = (fd_hash_table_entry*)A;
    fd_hash_table_entry* b = (fd_hash_table_entry*)B;

    return a->hash < b->hash;
}

bool s_equal_to_fd_table_entry(void* A, void* B)
{
    fd_hash_table_entry* a = (fd_hash_table_entry*)A;
    fd_hash_table_entry* b = (fd_hash_table_entry*)B;

    return a->hash == b->hash;
}
