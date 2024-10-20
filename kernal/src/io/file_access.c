#include "io/file_access.h"

#include "lib/dynamic_array.h"
#include "io/filesystem.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "lib/math.h"
#include "io/printf.h"
#include "io/sd.h"

#include <stdint.h>

struct file_discriptor_metadata
{
    uint32_t directory_lba; // The lba that the dirrectory is stored in
    uint32_t directory_lba_offset; // The offset (in sizeof(fat_directory_entry) bytes)
    uint32_t current_cluster_number;
    uint32_t first_cluster_number;
    uint32_t current_offset;
    size_t file_size_bytes;
    bool write_permissions;
};

typedef struct file_discriptor_metadata file_discriptor_metadata;

struct fd_hash_table_entry
{
    file_discriptor_metadata metadata;
    int hash;
};

typedef struct fd_hash_table_entry fd_hash_table_entry;

struct fat_entry_update
{
    uint32_t entry_number;
    uint32_t new_value;
};

typedef struct fat_entry_update fat_entry_update;

static dynamic_array s_fd_hash_table;
extern fat32_fs* root_file_system;

// Used by binary search function
static bool s_less_then_fd_table_entry(const void* A, const void* B);
static bool s_equal_to_fd_table_entry(const void* A, const void* B);

// Shearches the root dirrectory for a file
// @param path Null terminated string containing path
// @param directory_lba Stores the lba of the cluster the directory entry was found in
// @param directory_lba_offset The offset (in sizeof(fat_directory_entry) bytes)
// @return pointer to directory_entry structer or NULL if file does not exist
// @note path Does not start with '/' so "/a.txt" turns into "a.txt" (local paths are not supported)
static fat_directory_entry* s_find_file_from_path(const char* path, uint32_t* directory_lba, uint32_t* directory_lba_offset);

// Used by s_find_file_from_path to find a file
// @param path Path to file form current dirrectory
// @param current_dirrectory_cluster_number Cluster number of current dirrectory
// @param fat_buffer A 512 byte buffer to store a sector of the file allocation table into
// @param directory_lba Stores the lba of the cluster the directory entry was found in
// @param directory_lba_offset The offset (in sizeof(fat_directory_entry) bytes)
// @param dirrectory_cluster A 512 * sectors_per_cluster buffer to store the dirrectory into
// @return pointer to directory_entry structer or NULL if file does not exist
static fat_directory_entry* s_find_file_recursive(const char* path, uint32_t current_dirrectory_cluster_number, 
    uint32_t* fat_buffer, fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset);

// Used by s_find_file_recursive to get the correctly formated file name, also detects if we still are in a dirrectory
// @param path Path of the file
// @param buffer A 11 byte buffer to write into
// @return -1 if failed int32_max if file or the number of bytes taken (including traling /) if dirrectory
static int32_t s_format_file_name_8_3_standered(const char* path, char* buffer);

// Allocates new clusters to file after a write
// @param file Pointer to file discriptor of the file
// @param bytes_written bytes written to the file, (used to caculate new size)
// @param fat_buffer A 512 byte buffer to store a sector of the file allocation table into
// @param last_fat_sector Used to store the last sector of the fat that was loaded
// @return True if succesful or false if failed
static bool s_allocate_new_clusters_if_necessary(file_discriptor_metadata* file, size_t bytes_written, uint32_t* fat_buffer, uint32_t* last_fat_sector);

// Used to calcuate the number of btyes to be read / write from the 
// tail ends of a the clusters, and the ammount of clusters in the middle
// when reading / writting n bytes from the current offset
// @param file_discriptor_metadata A pointer to the file metadata
// @param n Number of bytes to read / writes
// @param bytes_from_first Set to the bytes to be read / write form the first cluster
// @param middle_clusters The number of clusters between the two tails
// @param bytes_from_last Set to the bytes to be read / write form the last cluster
static void s_cacluate_number_of_byte_from_tail_clusters_and_middle_clusters(file_discriptor_metadata* file, uint32_t n,
    uint32_t* bytes_from_first, uint32_t* middle_clusters, uint32_t* bytes_from_last);

// Find n free clusters and returns them as a list of updates
// @param n The number of clusters to get
// @param fat_buffer A pointer to a 512 byte buffer to load the FAT into
// @param last_fat_sector A pointer to a uint32_t  to store the last sector of the fat that was used
// @return the clusters in a fat_entry_update list connected to each other or NULL if failed
// @note The first entry of the returned list needs its "entry_number" set or use the return + 1 to if this is a new allocation
// the lest entry is all zeros to define the end
// @warning The returned value is allocated on the stack, Remember to free it
static fat_entry_update* s_find_free_clusters(uint32_t n, uint32_t* fat_buffer, uint32_t* last_fat_sector);

// Writes the given list of updates to the FAT
// @param updates The list of updates to use
// @param fat_buffer A pointer to a 512 byte buffer to load the FAT into
// @param last_fat_sector A pointer to a uint32_t  to store the last sector of the fat that was used
// @return True if success, false if failed
// @note The end of the list is signifed by a entry with "entry_number" or "new_value" set to 0
static bool s_write_fat_updates(fat_entry_update* updates, uint32_t* fat_buffer, uint32_t* last_fat_sector);

void initialize_file_access()
{
    initialize_dynamic_array(sizeof(fd_hash_table_entry), 0, &s_fd_hash_table);
}

int open(const char* path, int flags)
{
    if(path[0] == '/' || path[0] == '\\') 
        path++; // Just used so /a.txt becomes a.txt 

    int file_discriptor = (int)djb2_hash_uppercase(path);
    
    if (file_discriptor == -1)
        file_discriptor -= 1;

    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = file_discriptor;
 
    bool allready_opened = false;
    uint32_t directory_lba_offset = 0;
    uint32_t directory_lba = 0;

    int index = (int)dynamic_array_find_closest_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, &allready_opened,
        s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (allready_opened == true)
        return -1;

    

    fat_directory_entry* file = s_find_file_from_path(path, &directory_lba, &directory_lba_offset);

    if (file == NULL)
        return -1;

    bool is_readonly = file->attributes & FAT_DIRECTORY_ATTRIBUTES_READ_ONLY;

    fd_hash_table_entry entry;
    entry.metadata.first_cluster_number = (file->first_cluster_number_higher_16_bits << 16) | (file->first_cluster_number_lowwer_16_bits);
    entry.metadata.file_size_bytes = file->file_size_bytes;
    entry.metadata.current_cluster_number = entry.metadata.first_cluster_number;
    entry.metadata.write_permissions = flags & FILE_FLAGS_READ_WRITE;
    entry.metadata.directory_lba_offset = directory_lba_offset;
    entry.metadata.directory_lba = directory_lba;
    entry.metadata.current_offset = 0;
    entry.hash = file_discriptor;
    free(file);

    if (flags & FILE_FLAGS_READ_WRITE)
    {
        if (is_readonly)
        {
            return -1;
        }
    }
    else if (flags & FILE_FLAGS_UNUSED_BITS)
        return -1;


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

fat_directory_entry* s_find_file_recursive(const char* path, uint32_t current_dirrectory_cluster_number, uint32_t* fat_buffer, 
    fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset)
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
    uint32_t cluster_lba = 0;

    while (next_cluster < 0x0FFFFFF8)
    {   
        cluster_lba = root_file_system->data_sector + (next_cluster - 2) * root_file_system->number_of_sectors_per_cluster;
        
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
        if (entry == NULL)
            return NULL;

        memcpy(entry, matching_dirrectory, sizeof(fat_directory_entry));

        if (directory_lba != NULL)
            *directory_lba = cluster_lba;
        if(directory_lba_offset != NULL)
            *directory_lba_offset = (matching_dirrectory - dirrectory_cluster);

        return entry;
    }

    if (matching_dirrectory->attributes & FAT_DIRECTORY_ATTRIBUTES_DIRECTORY)
    {
        printf("Erorr: Expected dirrectory, found somethign else!\n");

        return NULL;
    }

    uint32_t next_dirrectory_cluster = (matching_dirrectory->first_cluster_number_higher_16_bits << 16) | 
        (matching_dirrectory->first_cluster_number_lowwer_16_bits);

    return s_find_file_recursive(path + name_offset, next_dirrectory_cluster, fat_buffer, dirrectory_cluster, directory_lba, directory_lba_offset);
}

fat_directory_entry* s_find_file_from_path(const char* path, uint32_t* directory_lba, uint32_t* directory_lba_offset)
{
    fat_directory_entry* dirrectory_cluster = malloc(512 * root_file_system->number_of_sectors_per_cluster);

    if (dirrectory_cluster == NULL)
        return NULL;

    uint32_t* fat_buffer = malloc(512);

    if (fat_buffer == NULL)
        return NULL;

    fat_directory_entry* entry = s_find_file_recursive(path, root_file_system->root_cluster, fat_buffer, 
        dirrectory_cluster, directory_lba, directory_lba_offset);

    free(dirrectory_cluster);
    free(fat_buffer);

    return entry;
}

void s_cacluate_number_of_byte_from_tail_clusters_and_middle_clusters(file_discriptor_metadata* file, uint32_t n,
    uint32_t* bytes_from_first, uint32_t* middle_clusters, uint32_t* bytes_from_last)
{
    const uint32_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;

    int64_t right_space_in_first_cluster = cluster_size - (file->current_offset % cluster_size);

    *bytes_from_first = min(right_space_in_first_cluster, n);

    if (right_space_in_first_cluster < n)
    {
        uint32_t left_over_bytes = n - right_space_in_first_cluster;
        *bytes_from_last = left_over_bytes % cluster_size;
        *middle_clusters = left_over_bytes / cluster_size;
    }
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

    const uint32_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;
    
    n = min(file->metadata.file_size_bytes - file->metadata.current_offset, n); // Make sure we dont try to read bytes that dont exist
    uint32_t number_of_bytes_to_read_from_first_clsuter = 0;
    uint32_t number_of_bytes_to_read_from_last_clsuter = 0;
    uint32_t number_of_middle_clusters = 0;

    s_cacluate_number_of_byte_from_tail_clusters_and_middle_clusters(&file->metadata, n,    // File and number of bytes
        &number_of_bytes_to_read_from_first_clsuter,                                        // return varibles
        &number_of_middle_clusters,
        &number_of_bytes_to_read_from_last_clsuter);

    uint8_t* file_temporay_buffer = NULL; // Used to copy sectors to when we're not copying the hole thing
    uint32_t cluster_lba = 0;
    uint8_t* buffer = (uint8_t*)buf;

    if (number_of_bytes_to_read_from_first_clsuter != 0 || number_of_bytes_to_read_from_last_clsuter != 0)
        file_temporay_buffer = malloc(cluster_size);

    if (file_temporay_buffer == NULL)
        return -1;
    
    if (number_of_bytes_to_read_from_first_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_readblock(cluster_lba, file_temporay_buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
        {
            printf("Error: Failed to read SD!\n");
            free(file_temporay_buffer);
            return -1;
        }

        memcpy(buffer, file_temporay_buffer + (file->metadata.current_offset % cluster_size), number_of_bytes_to_read_from_first_clsuter);
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
    uint32_t number_of_clusters_to_read = number_of_middle_clusters + (number_of_bytes_to_read_from_last_clsuter == 0) ? 0 : 1;

    if (fat_buffer == NULL)
    {
        if (file_temporay_buffer != NULL)
            free(file_temporay_buffer);

        return -1;
    }

    for ( ; number_of_clusters_to_read > 0; number_of_clusters_to_read--)
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

            if (sd_readblock(cluster_lba, buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
            {
                printf("Erorr: Failed to read SD!\n");
                if (file_temporay_buffer != NULL)
                    free(file_temporay_buffer);
                free (fat_buffer);

                return -1;
            }
            file->metadata.current_offset += cluster_size;
            buffer += cluster_size;
            
            number_of_middle_clusters--;
        }
    }

    free(fat_buffer);

    if (number_of_bytes_to_read_from_last_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_readblock(cluster_lba, file_temporay_buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
        {
            printf("Error: Failed to read SD!\n");
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

size_t write(int fd, const void* buf, size_t n)
{
    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = fd;

    size_t index = dynamic_array_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (index == -1)
        return -1;

    fd_hash_table_entry* file = (fd_hash_table_entry*)s_fd_hash_table.ptr;
    file += index;

    uint32_t last_fat_sector = UINT32_MAX;
    uint32_t* fat_buffer = malloc(512);
    if (fat_buffer == NULL)     // Handle allocation failed
        return -1;              // Very advanced methiod, give up


    if (!s_allocate_new_clusters_if_necessary(&file->metadata, n, fat_buffer, &last_fat_sector))
    {
        if (fat_buffer != NULL)
            free(fat_buffer);

        return -1;
    }
    
    const size_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;

    void* tempoary_buffer = malloc(cluster_size);

    if (tempoary_buffer == NULL)
    {
        if (fat_buffer != NULL)
            free(fat_buffer);

        return -1;
    }

    // Now rewrite the dirrectory entry

    if (sd_readblock(file->metadata.directory_lba, tempoary_buffer, 1) != 512)
    {
        printf("Error: Failed to %s dirrectory!\nWarning: File may have been courrpted!\n", "read");

        return -1;
    }

    (((fat_directory_entry*)tempoary_buffer) + file->metadata.directory_lba_offset)->file_size_bytes = file->metadata.file_size_bytes;

    if (sd_writeblock(file->metadata.directory_lba, tempoary_buffer, 1) != 512)
    {
        printf("Error: Failed to %s dirrectory!\nWarning: File may have been courrpted!\n", "write");

        return -1;
    }

    // Accutelly write the thing

    uint32_t number_of_bytes_to_write_to_first_clsuter = 0;
    uint32_t number_of_bytes_to_write_to_last_clsuter = 0;
    uint32_t number_of_middle_clusters = 0;

    s_cacluate_number_of_byte_from_tail_clusters_and_middle_clusters(&file->metadata, n,    // File and number of bytes
        &number_of_bytes_to_write_to_first_clsuter,                                         // return varibles
        &number_of_middle_clusters,
        &number_of_bytes_to_write_to_last_clsuter);

    uint32_t cluster_lba = 0;
    uint8_t* buffer = (uint8_t*)buf;

    if (number_of_bytes_to_write_to_first_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_write_section(cluster_lba,                           // LBA to write to
            buffer,                                                 // Bytes to write
            file->metadata.current_offset % cluster_size,           // Offset from start
            number_of_bytes_to_write_to_first_clsuter,             // Number of bytes
            root_file_system->number_of_sectors_per_cluster,        // Ammount of from the SD to work with
            tempoary_buffer) == 0)                                  // Working buffer
        {
            printf("Error: Failed to write to SD!\n");
            free(tempoary_buffer);

            if (fat_buffer != NULL)
                free(fat_buffer);

            return -1;
        }

        file->metadata.current_offset += number_of_bytes_to_write_to_first_clsuter;
        buffer += number_of_bytes_to_write_to_first_clsuter;
    }

    if (number_of_middle_clusters == 0 && number_of_bytes_to_write_to_last_clsuter == 0)
    {
        free(tempoary_buffer);

        if (fat_buffer != NULL)
            free(fat_buffer);

        return n;
    }

    if (fat_buffer == NULL)
        fat_buffer = malloc(512);

    if (fat_buffer == NULL)     // Handle if allocation failed
    {
        free(tempoary_buffer);
        return -1;
    }

    uint32_t number_of_clusters_to_write_to = number_of_middle_clusters + (number_of_bytes_to_write_to_last_clsuter == 0) ? 0 : 1;

    for ( ; number_of_clusters_to_write_to > 0; number_of_clusters_to_write_to--)
    {
        uint32_t fat_sector = root_file_system->first_fat_sector + (file->metadata.current_cluster_number / (512 / 4));
        uint32_t fat_offset = (file->metadata.current_cluster_number % (512 / 4));

        if (last_fat_sector != fat_sector)
        {
            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");
                free(tempoary_buffer);
                free (fat_buffer);

                return -1;
            }
            last_fat_sector = fat_sector;
        }

        uint32_t new_cluster_number = fat_buffer[fat_offset] & 0x0FFFFFFF;

        if (new_cluster_number >= 0x0FFFFFF8 && (number_of_middle_clusters != 0 || number_of_bytes_to_write_to_last_clsuter != 0))
        {
            printf("Erorr: Missmach between file size on dirrecotry entry and disk!\n");
            free(tempoary_buffer);
            free (fat_buffer);

            return -1;
        }
        file->metadata.current_cluster_number = new_cluster_number;

        if (number_of_middle_clusters > 0)
        {
            cluster_lba = root_file_system->data_sector;
            cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

            if (sd_writeblock(cluster_lba, buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
            {
                printf("Erorr: Failed to write to SD!\n");
                free(tempoary_buffer);
                free (fat_buffer);

                return -1;
            }
            file->metadata.current_offset += cluster_size;
            buffer += cluster_size;
            
            number_of_middle_clusters--;
        }
    }
    free(fat_buffer);


    if (number_of_bytes_to_write_to_last_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->metadata.current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_write_section(cluster_lba,                           // LBA to write to
            buffer,                                                 // Bytes to write
            0,                                                      // Offset from start
            number_of_bytes_to_write_to_last_clsuter,              // Number of bytes
            root_file_system->number_of_sectors_per_cluster,        // Ammount of from the SD to work with
            tempoary_buffer) == 0)                                  // Working buffer
        {
            printf("Failed to read SD!\n");
            free(tempoary_buffer);
            return -1;
        }
        file->metadata.current_offset += number_of_bytes_to_write_to_last_clsuter;
        buffer += number_of_bytes_to_write_to_last_clsuter;
    }


    free(tempoary_buffer);

    file->metadata.current_offset += n;

    return n;
}

bool s_allocate_new_clusters_if_necessary(file_discriptor_metadata* file, size_t bytes_written, uint32_t* fat_buffer, uint32_t* last_fat_sector)
{
    size_t new_size = file->current_offset + bytes_written;
    new_size = max(new_size, file->file_size_bytes);

    const size_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;

    uint32_t required_clusters = new_size / cluster_size + ((new_size % cluster_size) ? 1 : 0);
    uint32_t current_clusters = file->file_size_bytes / cluster_size + ((file->file_size_bytes % cluster_size) ? 1 : 0);

    if (current_clusters == 0)
        current_clusters++;

    uint32_t number_of_cluster_to_be_added = required_clusters - current_clusters;

    if (number_of_cluster_to_be_added == 0)
    {
        file->file_size_bytes = new_size;

        return true;
    }

    uint32_t current_cluster_number = file->current_cluster_number;

    while (current_cluster_number < 0x0FFFFFF8) // Skip to the last cluster of file
    {
        uint32_t fat_sector = root_file_system->first_fat_sector + (current_cluster_number / (512 / 4));
        uint32_t fat_offset = (current_cluster_number % (512 / 4));

        if (*last_fat_sector != fat_sector)
        {
            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");

                return false;
            }
            *last_fat_sector = fat_sector;
        }

        current_cluster_number = fat_buffer[fat_offset] & 0x0FFFFFFF;
    }

    // Now we need to find free sectors
    fat_entry_update* allocation_table_updates = s_find_free_clusters(number_of_cluster_to_be_added, fat_buffer, last_fat_sector);

    if (allocation_table_updates == NULL)
        return false;

    allocation_table_updates[0].entry_number = current_cluster_number; // Set the entry to the last cluster of the file
       
    bool success = s_write_fat_updates(allocation_table_updates, fat_buffer, last_fat_sector);
    free(allocation_table_updates);
    
    if (!success)
        return false;

    file->file_size_bytes = new_size;

    return true;
}

fat_entry_update* s_find_free_clusters(uint32_t n, uint32_t* fat_buffer, uint32_t* last_fat_sector)
{
    fat_entry_update* allocation_table_updates = malloc(sizeof(fat_entry_update) * (n + 2));
    if (allocation_table_updates == NULL)
        return NULL;

    memclr(allocation_table_updates, sizeof(fat_entry_update) * (n + 2));

    uint32_t fat_sector = root_file_system->first_fat_sector;
    uint32_t current_fat_sector_number = 0;
    uint32_t number_of_clusters_found = 0;
    
    while ((fat_sector < (root_file_system->first_fat_sector + root_file_system->sectors_per_fat)) 
            && (n > number_of_clusters_found))
    {
        if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
        {
            printf("Erorr: Failed to read FAT!\n");
            free(allocation_table_updates);

            return NULL;
        }
        *last_fat_sector = fat_sector;
        
        for (int i = 0; (i < (512 / 4)) && (n > number_of_clusters_found); i++)
        {
            if (fat_buffer[i] != 0)
                continue; // If not free just ignore and keep searching for one

            allocation_table_updates[number_of_clusters_found++].new_value = current_fat_sector_number * 4 + i;
            allocation_table_updates[number_of_clusters_found].entry_number = current_fat_sector_number * 4 + i;
        }

        current_fat_sector_number++;
        fat_sector++;
    }

    return allocation_table_updates;
}

bool s_write_fat_updates(fat_entry_update* updates, uint32_t* fat_buffer, uint32_t* last_fat_sector)
{   
    bool is_first_entry = true;


    for ( ; (updates->entry_number != 0) && (updates->new_value != 0); updates++)
    {   
        uint32_t current_cluster_number = updates->entry_number;
        uint32_t fat_sector = root_file_system->first_fat_sector + (current_cluster_number / (512 / 4));
        uint32_t fat_offset = (current_cluster_number % (512 / 4));

        if (*last_fat_sector != fat_sector)
        {
            // We dont need to write on the first entry as no changes have been made yet
            if (!is_first_entry) 
            {
                if (sd_writeblock(*last_fat_sector, fat_buffer, 1) != 512)
                {
                    printf("Erorr: Failed to write to FAT!\n");
                    printf("Warning: allocation table may have been courrpted!\n");
                    // free(allocation_table_updates);
                    return false;
                }
            }

            // We also check this here as it stops a uneeded read
            // We have reached the end of the updates we can make
            if (updates->entry_number == 0 ||
                updates->new_value == 0)
                break;

            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");
                // free(allocation_table_updates);
                return false;
            }

            *last_fat_sector = fat_sector;
        }

        // We have reached the end of the updates we can make
        if (updates->entry_number == 0 ||
                updates->new_value == 0)
                break;

        fat_buffer[fat_offset] = updates->new_value;

        is_first_entry = true;
    }

    return true;
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

    const uint32_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;

    uint32_t old_cluster_offset = file->metadata.current_offset / cluster_size;
    uint32_t new_cluster_offset = new_offset / cluster_size;
    
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

    if (fat_buffer == NULL)
        return -1;

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

bool s_less_then_fd_table_entry(const void* A, const void* B)
{
    fd_hash_table_entry* a = (fd_hash_table_entry*)A;
    fd_hash_table_entry* b = (fd_hash_table_entry*)B;

    return a->hash < b->hash;
}

bool s_equal_to_fd_table_entry(const void* A, const void* B)
{
    fd_hash_table_entry* a = (fd_hash_table_entry*)A;
    fd_hash_table_entry* b = (fd_hash_table_entry*)B;

    return a->hash == b->hash;
}
