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
    bool is_owened_by_user;
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

static uint32_t last_fat_sector = UINT32_MAX;
static uint32_t fat_buffer[512];
static bool s_is_in_user_mode = false;

static dynamic_array s_fd_hash_table;
extern fat32_fs* root_file_system;

// Used by binary search function
static bool s_less_then_fd_table_entry(const void* A, const void* B);
static bool s_equal_to_fd_table_entry(const void* A, const void* B);

// Shearches the root dirrectory for a file
// @param path Null terminated string containing path
// @param can_create_new_entrys If true the function will create new entrys so the file exists
// @param directory_lba Stores the lba of the sector the directory entry was found in
// @param directory_lba_offset The offset (in sizeof(fat_directory_entry) bytes)
// @return pointer to directory_entry structer or NULL if file does not exist
// @note path Does not start with '/' so "/a.txt" turns into "a.txt" (local paths are not supported)
static fat_directory_entry* s_find_file_from_path(const char* path, bool can_create_new_entrys, uint32_t* directory_lba, uint32_t* directory_lba_offset);

// Used by s_find_file_from_path to find a file
// @param path Path to file form current dirrectory
// @param current_dirrectory_cluster_number Cluster number of current dirrectory
// @param can_create_new_entrys If true the function will create new entrys so the file exists
// @param fat_buffer A 512 byte buffer to store a sector of the file allocation table into
// @param directory_lba Stores the lba of the sector the directory entry was found in
// @param directory_lba_offset The offset (in sizeof(fat_directory_entry) bytes)
// @param dirrectory_cluster A 512 * sectors_per_cluster buffer to store the dirrectory into
// @return pointer to directory_entry structer or NULL if file does not exist
static fat_directory_entry* s_find_file_recursive(const char* path, uint32_t current_dirrectory_cluster_number, bool can_create_new_entrys, 
    uint32_t* fat_buffer, fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset);

// Used by s_find_file_recursive to get the correctly formated file name, also detects if we still are in a dirrectory
// @param path Path of the file
// @param buffer A 11 byte buffer to write into
// @return -1 if failed int32_max if file or the number of bytes taken (including traling /) if dirrectory
static int32_t s_format_file_name_8_3_standered(const char* path, char* buffer);

// Allocates new clusters to file after a write
// @param file Pointer to file discriptor of the file
// @param bytes_written bytes written to the file, (used to caculate new size)
// @return True if succesful or false if failed
static bool s_allocate_new_clusters_if_necessary(file_discriptor_metadata* file, size_t bytes_written);

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
// @return the clusters in a fat_entry_update list connected to each other or NULL if failed
// @note The first entry of the returned list needs its "entry_number" set or use the return + 1 to if this is a new allocation
// the lest entry is all zeros to define the end
// @warning The returned value is allocated on the stack, Remember to free it
static fat_entry_update* s_find_free_clusters(uint32_t n);

// Writes the given list of updates to the FAT
// @param updates The list of updates to use
// @return True if success, false if failed
// @note The end of the list is signifed by a entry with "entry_number" set to UINT32_MAX
static bool s_write_fat_updates(fat_entry_update* updates);

// Resizes a file to new_size bytes but does not zero or anything just sets up the clusters and the dirrectory entry
// @param file Pointer to file discriptor of the file
// @param new_size bytes written to the file
// @return True if succesful or false if failed
// static bool s_resize_file(file_discriptor_metadata* file, size_t new_size);

// Expands a file to new_size bytes but does not zero or anything just sets up the clusters and the dirrectory entry
// Inputs and outputs the same as s_resize_file
static bool s_expand_file(file_discriptor_metadata* file, size_t new_size);

// Same as s_expand_file but doesn't change the dirrectory entry for the file
static bool s_expand_file_internal(file_discriptor_metadata* file, size_t new_size);

// Shinks a file to new_size bytes, sets up the clusters and the dirrectory entry
// Inputs and outputs the same as s_resize_file except for the addation of 
static bool s_shink_file(file_discriptor_metadata* file, size_t new_size);

// Same as s_shink_file but doesn't change the dirrectory entry for the file except for how it handles a size of zero
// @warning When s_shink_file(file, 0) is called s_shink_file_internal(file, 1) is called insted.
//          When calling s_shink_file_internal(file, 0) The first cluster is removed
//          and should only be used when deleting a file.
static bool s_shink_file_internal(file_discriptor_metadata* file, size_t new_size);

// Removes a file, same as fremove but uses file_discriptor_metadata* 
// @warning This function does not close the file
static int s_fremove_internal(file_discriptor_metadata* file);

// Sets the values of the file in its discriptor / dirrectory entry, to what the struct has stored
// @param file The file to set and the metedata to store
// @param working_buffer (Minium size of 512 bytes stores the memory used to work with the disk
// @return True if succesful or false if failed
// @note If first_cluster_number is set to UINT32_MAX the entry is deleteted
static bool s_update_file_discriptor(file_discriptor_metadata* file, uint32_t* working_buffer);

// Same as lseek but using file_discriptor_metadata insted of file discriptors them self
// @param file The file to work with
// @param fat_buffer A 512 byte buffer to store a sector of the file allocation table into
// @param last_fat_sector Used to store the last sector of the fat that was loaded
static ptrdiff_t s_lseek_internal(file_discriptor_metadata* file, ptrdiff_t offset, int whence);

// Creates a new entry in a dirrectory allocating clusters for the entry and new cluster for the dirrectory if needed
// @param dirrectory_cluster_number The cluster number of the dirrectoruy we want to instert into
// @param name The 8.3 staneded name of the entry
// @param is_dirrectory If set to true the entry will be another dirrectory (adds the FAT_DIRECTORY_ATTRIBUTES_DIRECTORY flag
//                      and adds the "." and ".." entrys
// @param dirrectory_cluster A buffer the size of a cluster on the disk to be used as working memory
// @param directory_lba Stores the lba of the sector the directory entry was found in
// @param directory_lba_offset The offset (in sizeof(fat_directory_entry) bytes)
// @return A (heap allocated) copy of the directory entry or NULL if failed
static fat_directory_entry* s_create_new_dirrectory_entry(uint32_t dirrectory_cluster_number, const char* name, bool is_dirrectory, 
    fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset);

// Writes the given entry to the given dirrectory, and allocates new clusters to the dirrectory if needed
// @param dirrectory_cluster_number The cluster number of the dirrectoruy we want to instert into
// @param entry The 8.3 staneded name of the entry
// @param dirrectory_cluster A buffer the size of a cluster on the disk to be used as working memory
// @param directory_lba Stores the lba of the sector the directory entry was found in
// @param directory_lba_offset The offset (in sizeof(fat_directory_entry) bytes)
// @return True if success False if failed
static bool s_write_new_dirrectory_entry(uint32_t dirrectory_cluster_number, const fat_directory_entry* entry, 
    fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset);

// Same as lseek but using file_discriptor_metadata insted of file discriptors them 
static size_t s_write_internal(file_discriptor_metadata* file, const void* buf, size_t n);

// Shearches s_fd_hash_table for fd and returns the metadata
// @return Pointer to the meta data / NULL if failed
// @note Pointer doesn't need to be freed
static file_discriptor_metadata* s_get_file_metadata_from_discriptor(int fd);

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

    bool can_create_new_entrys = (flags & FILE_FLAGS_CREATE) && (flags & FILE_FLAGS_READ_WRITE);

    fat_directory_entry* file = s_find_file_from_path(path, can_create_new_entrys, &directory_lba, &directory_lba_offset);

    if (file == NULL)
        return -1;

    bool is_readonly = file->attributes & FAT_DIRECTORY_ATTRIBUTES_READ_ONLY;
    bool can_write = (!is_readonly) && (flags & FILE_FLAGS_READ_WRITE);

    fd_hash_table_entry entry;
    memclr(&entry, sizeof(fd_hash_table_entry));

    entry.metadata.first_cluster_number = (file->first_cluster_number_higher_16_bits << 16) | (file->first_cluster_number_lowwer_16_bits);
    entry.metadata.file_size_bytes = file->file_size_bytes;
    entry.metadata.current_cluster_number = entry.metadata.first_cluster_number;
    entry.metadata.write_permissions = flags & FILE_FLAGS_READ_WRITE;
    entry.metadata.directory_lba_offset = directory_lba_offset;
    entry.metadata.is_owened_by_user =s_is_in_user_mode;
    entry.metadata.directory_lba = directory_lba;
    entry.metadata.current_offset = 0;
    entry.hash = file_discriptor;
    free(file);

    if (flags & FILE_FLAGS_READ_WRITE)
    {
        if (is_readonly)
        {
            return -1;                          // Cant write to a read only file
        }
    }
    else if (flags & FILE_FLAGS_UNUSED_BITS)
        return -1;                              // Bad flags

    if (flags & FILE_FLAGS_APPEND)
    {
        if (!can_write)
            return -1;  

        if (s_lseek_internal(&entry.metadata, 0, SEEK_END) == -1)
            return -1;                          // Failded to seek
    }

    if (flags & FILE_FLAGS_TRUNCATE)
    {
        if (!can_write)
            return -1;  

        if (s_shink_file(&entry.metadata, 0) == false)
            return -1;                          // Failded to truncate
    }


    if (insert_dynamic_array(&entry, (size_t)index, &s_fd_hash_table) == false)
        return -1; // Failed to insert

    return file_discriptor;
}

size_t read(int fd, void* buf, size_t n)
{
    file_discriptor_metadata* file = s_get_file_metadata_from_discriptor(fd);

    if (file == NULL)
        return -1;

    const uint32_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;
    
    n = min(file->file_size_bytes - file->current_offset, n); // Make sure we dont try to read bytes that dont exist
    uint32_t number_of_bytes_to_read_from_first_clsuter = 0;
    uint32_t number_of_bytes_to_read_from_last_clsuter = 0;
    uint32_t number_of_middle_clusters = 0;

    s_cacluate_number_of_byte_from_tail_clusters_and_middle_clusters(file, n,    // File and number of bytes
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
        cluster_lba += (file->current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_readblock(cluster_lba, file_temporay_buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
        {
            printf("Error: Failed to read SD!\n");
            free(file_temporay_buffer);
            return -1;
        }

        memcpy(buffer, file_temporay_buffer + (file->current_offset % cluster_size), number_of_bytes_to_read_from_first_clsuter);
        file->current_offset += number_of_bytes_to_read_from_first_clsuter;
        buffer += number_of_bytes_to_read_from_first_clsuter;
    }

    if (number_of_middle_clusters == 0 && number_of_bytes_to_read_from_last_clsuter == 0)
    {
        if (file_temporay_buffer != NULL)
            free(file_temporay_buffer);

        return (file->file_size_bytes == file->current_offset) ? 0 : n;
    }

    uint32_t number_of_clusters_to_read = number_of_middle_clusters + (number_of_bytes_to_read_from_last_clsuter == 0) ? 0 : 1;

    for ( ; number_of_clusters_to_read > 0; number_of_clusters_to_read--)
    {
        uint32_t fat_sector = root_file_system->first_fat_sector + (file->current_cluster_number / (512 / 4));
        uint32_t fat_offset = (file->current_cluster_number % (512 / 4));

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
        file->current_cluster_number = new_cluster_number;

        if (number_of_middle_clusters > 0)
        {
            cluster_lba = root_file_system->data_sector;
            cluster_lba += (file->current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

            if (sd_readblock(cluster_lba, buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
            {
                printf("Erorr: Failed to read SD!\n");
                if (file_temporay_buffer != NULL)
                    free(file_temporay_buffer);
                free (fat_buffer);

                return -1;
            }
            file->current_offset += cluster_size;
            buffer += cluster_size;
            
            number_of_middle_clusters--;
        }
    }

    if (number_of_bytes_to_read_from_last_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_readblock(cluster_lba, file_temporay_buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
        {
            printf("Error: Failed to read SD!\n");
            free(file_temporay_buffer);
            return -1;
        }

        memcpy(buffer, file_temporay_buffer, number_of_bytes_to_read_from_last_clsuter);
        file->current_offset += number_of_bytes_to_read_from_last_clsuter;
        buffer += number_of_bytes_to_read_from_last_clsuter;
    }


    if (file_temporay_buffer != NULL)
        free(file_temporay_buffer);

    file->current_offset += n;

    return (file->file_size_bytes == file->current_offset) ? 0 : n;
}

size_t write(int fd, const void* buf, size_t n)
{
    file_discriptor_metadata* file = s_get_file_metadata_from_discriptor(fd);

    if (file == NULL)
        return -1;

    return s_write_internal(file, buf, n);
}

ptrdiff_t lseek(int fd, ptrdiff_t offset, int whence)
{
    file_discriptor_metadata* file = s_get_file_metadata_from_discriptor(fd);

    if (file == NULL)
        return -1;

    return s_lseek_internal(file, offset, whence);
}

int truncate(const char* path, size_t new_size)
{
    int fd = open(path, FILE_FLAGS_READ_WRITE);

    if (fd == -1)
        return -1;

    int success = ftruncate(fd, new_size);

    close(fd);

    return success;
}

int ftruncate(int fd, size_t new_size)
{
    file_discriptor_metadata* file = s_get_file_metadata_from_discriptor(fd);

    if (file == NULL)
        return -1;

    if (!file->write_permissions)
        return -1;              


    size_t old_size = file->file_size_bytes;

    bool success = false;

    if (new_size < old_size)    // Handle seek pointer
    {                           // Reset if file was shrunk
        success = s_shink_file(file, new_size);

        if (file->current_offset >= new_size)
        {
            file->current_cluster_number = file->first_cluster_number;
            file->current_offset = 0;

        }

        return success ? 0 : -1;
    }

    // Now zero extra bytes
    // This isn't the best methiod tbh but yea
    uint32_t seek_cluster = file->current_cluster_number;
    uint32_t seek_offset = file->current_offset;

    void* buffer = malloc(new_size - old_size);
    memclr(buffer, new_size - old_size);

    success = s_lseek_internal(file, 0, SEEK_END) != -1;

    success = s_write_internal(file, buffer, new_size - old_size) == (new_size - old_size);
    free(buffer);

    file->current_cluster_number = seek_cluster;
    file->current_offset = seek_offset;

    return success ? 0 : -1;
}

int remove(const char* path)
{
    int fd = open(path, FILE_FLAGS_READ_WRITE);

    if (fd == -1)
        return -1;

    int success = fremove(fd);

    return success;
}

int fremove(int fd)
{
   file_discriptor_metadata* file = s_get_file_metadata_from_discriptor(fd);

    if (file == NULL)
        return -1;

    if (file->write_permissions == false)
        return -1;

    int return_value = s_fremove_internal(file);
    close(fd);

    return return_value;
}

size_t get_file_size(int fd)
{
    file_discriptor_metadata* file = s_get_file_metadata_from_discriptor(fd);

    if (file == NULL)
        return -1;

    return file->file_size_bytes;
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

    if (entry->metadata.is_owened_by_user != s_is_in_user_mode)
        return -1;

    remove_dynamic_array_entry(index, &s_fd_hash_table);

    return 0;
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

fat_directory_entry* s_find_file_recursive(const char* path, uint32_t current_dirrectory_cluster_number, bool can_create_new_entrys,
    uint32_t* fat_buffer, fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset)
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
    uint32_t next_dirrectory_cluster = 0;
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


    if (matching_dirrectory == NULL)
    {
        if (!can_create_new_entrys)
            return NULL;
        // It doesn't exist but we can make it

        matching_dirrectory = s_create_new_dirrectory_entry(current_dirrectory_cluster_number,  // Cluster the dirrectory starts at
        name_buffer,                                                                            // 8.3 Name
        name_offset != INT32_MAX,                                                               // Is dirrectory
        dirrectory_cluster,                                                                     // Working buffer
        directory_lba, directory_lba_offset);                                                   // Offsets

        if (name_offset == INT32_MAX) // Is File
            return matching_dirrectory; // It allready is a copy soo...
        // Do it here so we free the copy
        
        next_dirrectory_cluster = (matching_dirrectory->first_cluster_number_higher_16_bits << 16) | 
        (matching_dirrectory->first_cluster_number_lowwer_16_bits);
        free(matching_dirrectory);

        return s_find_file_recursive(path + name_offset, next_dirrectory_cluster, can_create_new_entrys, 
            fat_buffer, dirrectory_cluster, directory_lba, directory_lba_offset);
    }

    if (matching_dirrectory->attributes == FAT_DIRECTORY_ATTRIBUTES_LFN)
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

        uint32_t offset = (matching_dirrectory - dirrectory_cluster);

        if (directory_lba != NULL)
            *directory_lba = cluster_lba + (offset / (512 / sizeof(fat_directory_entry)));
        if(directory_lba_offset != NULL)
            *directory_lba_offset = offset % (512 / sizeof(fat_directory_entry));

        return entry;
    }

    if (!(matching_dirrectory->attributes & FAT_DIRECTORY_ATTRIBUTES_DIRECTORY))
    {
        printf("Erorr: Expected dirrectory, found something else!\n");

        return NULL;
    }

    next_dirrectory_cluster = (matching_dirrectory->first_cluster_number_higher_16_bits << 16) | 
        (matching_dirrectory->first_cluster_number_lowwer_16_bits);

    return s_find_file_recursive(path + name_offset, next_dirrectory_cluster, can_create_new_entrys, 
        fat_buffer, dirrectory_cluster, directory_lba, directory_lba_offset);
}

fat_directory_entry* s_find_file_from_path(const char* path, bool can_create_new_entrys, uint32_t* directory_lba, uint32_t* directory_lba_offset)
{
    fat_directory_entry* dirrectory_cluster = malloc(512 * root_file_system->number_of_sectors_per_cluster);

    if (dirrectory_cluster == NULL)
        return NULL;

    fat_directory_entry* entry = s_find_file_recursive(path, root_file_system->root_cluster, can_create_new_entrys, 
        fat_buffer, dirrectory_cluster, directory_lba, directory_lba_offset);

    free(dirrectory_cluster);

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

// bool s_resize_file(file_discriptor_metadata* file, size_t new_size)
// {   
//     if (file->file_size_bytes == new_size)
//         return true;
//
//     if (file->file_size_bytes < new_size)
//         return s_expand_file(file, new_size);
//
//     return s_shink_file(file, new_size);
// }

bool s_expand_file(file_discriptor_metadata* file, size_t new_size)
{
    if (file->write_permissions == false || file->file_size_bytes > new_size)
        return false;

    bool success = s_expand_file_internal(file, new_size);

    if (!success)
        return false;

    file->file_size_bytes = new_size;
    void* file_io_working_buffer = malloc(512);
    success = s_update_file_discriptor(file, file_io_working_buffer);
    free(file_io_working_buffer);

    return success;
}

bool s_expand_file_internal(file_discriptor_metadata* file, size_t new_size)
{
    if (file->write_permissions == false || file->file_size_bytes > new_size)
        return false;

    const size_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;
    bool success = false;

    uint32_t required_clusters = new_size / cluster_size + ((new_size % cluster_size) ? 1 : 0);
    uint32_t current_clusters = file->file_size_bytes / cluster_size + ((file->file_size_bytes % cluster_size) ? 1 : 0);

    if (current_clusters == 0)
        current_clusters++;

    uint32_t number_of_cluster_to_be_added = required_clusters - current_clusters;

    if (number_of_cluster_to_be_added == 0)
        return true;

    uint32_t seek_cluster = file->current_cluster_number;
    uint32_t seek_offset = file->current_offset;

    if (s_lseek_internal(file, 0, SEEK_END) == -1)
        return false;

    uint32_t last_cluster = file->current_cluster_number;
    file->current_cluster_number = seek_cluster;
    file->current_offset = seek_offset;

    // Now we need to find free sectors
    fat_entry_update* allocation_table_updates = s_find_free_clusters(number_of_cluster_to_be_added);

    if (allocation_table_updates == NULL)
        return false;
    allocation_table_updates[0].entry_number = last_cluster; // Set the entry to the last cluster of the file
       
    success = s_write_fat_updates(allocation_table_updates);
    free(allocation_table_updates);
    
    return success;
}

bool s_shink_file(file_discriptor_metadata* file, size_t new_size)
{
    if (file->write_permissions == false || file->file_size_bytes < new_size)
        return false;

    if (new_size == 0)
        new_size = 1;

    bool success = s_shink_file_internal(file, new_size);
    
    if (!success)
        return false;

    file->file_size_bytes = new_size;
    void* file_io_working_buffer = malloc(512);
    success = s_update_file_discriptor(file, file_io_working_buffer);
    free(file_io_working_buffer);
    return success;
}

static bool s_shink_file_internal(file_discriptor_metadata* file, size_t new_size)
{
     if (file->write_permissions == false || file->file_size_bytes < new_size)
        return false;

    const size_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;

    uint32_t required_clusters = new_size / cluster_size + ((new_size % cluster_size) ? 1 : 0);
    uint32_t current_clusters = file->file_size_bytes / cluster_size + ((file->file_size_bytes % cluster_size) ? 1 : 0);

    uint32_t number_of_cluster_to_be_removed =  current_clusters - required_clusters;

    if (number_of_cluster_to_be_removed == 0)
        return true;
    
    uint32_t seek_cluster = file->current_cluster_number;
    uint32_t seek_offset = file->current_offset;

    if (s_lseek_internal(file, new_size, SEEK_SET) == -1)
        return false;
    
    uint32_t current_cluster_number = file->current_cluster_number;

    file->current_cluster_number = seek_cluster;
    file->current_offset = seek_offset;

    fat_entry_update* allocation_table_updates = malloc(sizeof(fat_entry_update) * (number_of_cluster_to_be_removed + 2));
    if (allocation_table_updates == NULL)
        return NULL;

    memclr(allocation_table_updates, sizeof(fat_entry_update) * (number_of_cluster_to_be_removed + 2));
    
    allocation_table_updates[0].entry_number = seek_cluster;    // Set the entry to the last cluster of the file
    allocation_table_updates[1].entry_number = UINT32_MAX;      // Used when only one cluster is edited and new_size is zero
    if (new_size != 0) 
        allocation_table_updates[0].new_value = 0x0FFFFFF8;         // And its the new end of the file
    
    if (new_size != 0 || (new_size == 0 && current_clusters > 1))
    {
        uint32_t i = 1;

        if (new_size != 0)
            number_of_cluster_to_be_removed++; // This way we skip the fisrt cluster

        while (number_of_cluster_to_be_removed-- > 0)
        {
            uint32_t fat_sector = root_file_system->first_fat_sector + (current_cluster_number / (512 / 4));
            uint32_t fat_offset = (current_cluster_number % (512 / 4));

            if (last_fat_sector != fat_sector)
            {
                if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
                {
                    printf("Erorr: Failed to read FAT!\n");

                    free(allocation_table_updates);
                    return false;
                }
                last_fat_sector = fat_sector;
            }

            current_cluster_number = fat_buffer[fat_offset] & 0x0FFFFFFF;

            bool is_last_cluster = current_cluster_number >= 0x0FFFFFF8;

            if ((is_last_cluster && number_of_cluster_to_be_removed != 0) ||
                (!is_last_cluster && number_of_cluster_to_be_removed == 0))
            {
                printf("Erorr: Missmach between file size on dirrecotry entry and disk!\n");

                free(allocation_table_updates);
                return false;
            }

            if (is_last_cluster)
                break;

            allocation_table_updates[i++].entry_number = current_cluster_number;
        }
        allocation_table_updates[i].entry_number = UINT32_MAX;
    }
    
       
    bool success = s_write_fat_updates(allocation_table_updates);
    free(allocation_table_updates);
    
    return success;
}

ptrdiff_t s_lseek_internal(file_discriptor_metadata* file, ptrdiff_t offset, int whence)
{
    ptrdiff_t new_offset = (ptrdiff_t)file->current_offset;

    switch (whence)
    {
    case SEEK_SET:
        new_offset = offset;
        break;
    case SEEK_CUR:
        new_offset += offset;
        break;
    case SEEK_END:
        new_offset = ((ptrdiff_t)file->file_size_bytes) + offset;
        break;
    default:
        return -1;
    }
    

    if (new_offset < 0 || new_offset > file->file_size_bytes)
        return -1;

    const uint32_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;

    uint32_t old_cluster_offset = file->current_offset / cluster_size;
    uint32_t new_cluster_offset = new_offset / cluster_size;
    
    if (new_cluster_offset == old_cluster_offset)
    {
        file->current_offset = new_offset;
        return new_offset;
    }

    uint32_t number_of_clusters_skip = 0;
    uint32_t current_cluster_number = 0;

    if (new_cluster_offset > old_cluster_offset)
    {
        number_of_clusters_skip = new_cluster_offset - old_cluster_offset;
        current_cluster_number = file->current_cluster_number;
    }
    else
    {
        number_of_clusters_skip = new_cluster_offset;
        current_cluster_number = file->first_cluster_number;
    }

    while (number_of_clusters_skip > 0)
    {
        uint32_t fat_sector = root_file_system->first_fat_sector + (current_cluster_number / (512 / 4));
        uint32_t fat_offset = (current_cluster_number % (512 / 4));

        if (last_fat_sector != fat_sector)
        {
            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");

                return -1;
            }
            last_fat_sector = fat_sector;
        }

        current_cluster_number = fat_buffer[fat_offset] & 0x0FFFFFFF;

        if (current_cluster_number >= 0x0FFFFFF8)
        {
            printf("Erorr: Missmach between file size on dirrecotry entry and disk!\n");

            return -1;
        }
        

        number_of_clusters_skip--;
    }

    file->current_cluster_number = current_cluster_number;
    file->current_offset = new_offset;
    return new_offset;
}

size_t s_write_internal(file_discriptor_metadata* file, const void* buf, size_t n)
{
    if (!file->write_permissions)
        return -1;

    if (!s_allocate_new_clusters_if_necessary(file, n))
        return -1;
    
    const size_t cluster_size = root_file_system->number_of_sectors_per_cluster * 512;

    void* tempoary_buffer = malloc(cluster_size);

    if (tempoary_buffer == NULL)
        return -1;

    // Accutelly write the thing

    uint32_t number_of_bytes_to_write_to_first_clsuter = 0;
    uint32_t number_of_bytes_to_write_to_last_clsuter = 0;
    uint32_t number_of_middle_clusters = 0;

    s_cacluate_number_of_byte_from_tail_clusters_and_middle_clusters(file, n,    // File and number of bytes
        &number_of_bytes_to_write_to_first_clsuter,                                         // return varibles
        &number_of_middle_clusters,
        &number_of_bytes_to_write_to_last_clsuter);

    uint32_t cluster_lba = 0;
    uint8_t* buffer = (uint8_t*)buf;

    if (number_of_bytes_to_write_to_first_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

        if (sd_write_section(cluster_lba,                           // LBA to write to
            buffer,                                                 // Bytes to write
            file->current_offset % cluster_size,           // Offset from start
            number_of_bytes_to_write_to_first_clsuter,             // Number of bytes
            root_file_system->number_of_sectors_per_cluster,        // Ammount of from the SD to work with
            tempoary_buffer) == 0)                                  // Working buffer
        {
            printf("Error: Failed to write to SD!\n");
            free(tempoary_buffer);

            return -1;
        }

        file->current_offset += number_of_bytes_to_write_to_first_clsuter;
        buffer += number_of_bytes_to_write_to_first_clsuter;
    }

    if (number_of_middle_clusters == 0 && number_of_bytes_to_write_to_last_clsuter == 0)
    {
        free(tempoary_buffer);

        return n;
    }

    uint32_t number_of_clusters_to_write_to = number_of_middle_clusters + (number_of_bytes_to_write_to_last_clsuter == 0) ? 0 : 1;

    for ( ; number_of_clusters_to_write_to > 0; number_of_clusters_to_write_to--)
    {
        uint32_t fat_sector = root_file_system->first_fat_sector + (file->current_cluster_number / (512 / 4));
        uint32_t fat_offset = (file->current_cluster_number % (512 / 4));

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
        file->current_cluster_number = new_cluster_number;

        if (number_of_middle_clusters > 0)
        {
            cluster_lba = root_file_system->data_sector;
            cluster_lba += (file->current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

            if (sd_writeblock(cluster_lba, buffer, root_file_system->number_of_sectors_per_cluster) != cluster_size)
            {
                printf("Erorr: Failed to write to SD!\n");
                free(tempoary_buffer);
                free (fat_buffer);

                return -1;
            }
            file->current_offset += cluster_size;
            buffer += cluster_size;
            
            number_of_middle_clusters--;
        }
    }

    if (number_of_bytes_to_write_to_last_clsuter != 0)
    {
        cluster_lba = root_file_system->data_sector;
        cluster_lba += (file->current_cluster_number - 2) * root_file_system->number_of_sectors_per_cluster;

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
        file->current_offset += number_of_bytes_to_write_to_last_clsuter;
        buffer += number_of_bytes_to_write_to_last_clsuter;
    }


    free(tempoary_buffer);

    file->current_offset += n;

    return n;
}

int s_fremove_internal(file_discriptor_metadata* file)
{
    if (file->write_permissions == false)
        return -1;

    if (!s_shink_file_internal(file, 0))
        return -1;

    file->first_cluster_number = UINT32_MAX;

    void* working_buffer = malloc(512);

    if (working_buffer == NULL)
    {
        printf("Failed to remove file's dirrectory entry but FAT entrys have been changed!\n");
        return -1;
    }

    bool success = s_update_file_discriptor(file, working_buffer);
    free(working_buffer);

    if (!success)
    {
        printf("Failed to remove file's dirrectory entry but FAT entrys have been changed!\n");
        return -1;
    }

    return 0;
}

bool s_allocate_new_clusters_if_necessary(file_discriptor_metadata* file, size_t bytes_written)
{
    size_t new_size = file->current_offset + bytes_written;

    if (new_size > file->file_size_bytes)
        return s_expand_file(file, new_size);
    
    return true;
}

bool s_update_file_discriptor(file_discriptor_metadata* file, uint32_t* working_buffer)
{
    if (file->write_permissions == false)
        return false;

    if (sd_readblock(file->directory_lba, working_buffer, 1) != 512)
    {
        printf("Error: Failed to %s dirrectory!\nWarning: File may have been courrpted!\n", "read");

        return false;
    }

    fat_directory_entry* entry = ((fat_directory_entry*)working_buffer) + file->directory_lba_offset;

    if (file->first_cluster_number == UINT32_MAX)   // If the cluster number is UINT32_MAX we delete the entry K?
    {
        memclr(entry, sizeof(fat_directory_entry));
    }
    else                                            // Just update it
        entry->file_size_bytes = file->file_size_bytes;


    if (sd_writeblock(file->directory_lba, working_buffer, 1) != 512)
    {
        printf("Error: Failed to %s dirrectory!\nWarning: File may have been courrpted!\n", "write");

        return false;
    }

    return true;
}

fat_entry_update* s_find_free_clusters(uint32_t n)
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
        if ((last_fat_sector != fat_sector) && (sd_readblock(fat_sector, fat_buffer, 1) != 512))
        {
            printf("Erorr: Failed to read FAT!\n");
            free(allocation_table_updates);

            return NULL;
        }
        last_fat_sector = fat_sector;
        
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

    allocation_table_updates[number_of_clusters_found++].new_value = 0x0FFFFFF8;
    allocation_table_updates[number_of_clusters_found].entry_number = UINT32_MAX;

    return allocation_table_updates;
}

bool s_write_fat_updates(fat_entry_update* updates)
{   
    bool is_first_entry = true;


    while (1)
    {   
        uint32_t current_cluster_number = updates->entry_number;
        uint32_t fat_sector = root_file_system->first_fat_sector + (current_cluster_number / (512 / 4));
        uint32_t fat_offset = (current_cluster_number % (512 / 4));

        if (last_fat_sector != fat_sector)
        {
            // We dont need to write on the first entry as no changes have been made yet
            if (!is_first_entry) 
            {
                if (sd_writeblock(last_fat_sector, fat_buffer, 1) != 512)
                {
                    printf("Erorr: Failed to write to FAT!\n");
                    printf("Warning: allocation table may have been courrpted!\n");
                    return false;
                }
            }

            // We also check this here as it stops a uneeded read
            // We have reached the end of the updates we can make
            if (updates->entry_number == UINT32_MAX)
                break;

            if (sd_readblock(fat_sector, fat_buffer, 1) != 512)
            {
                printf("Erorr: Failed to read FAT!\n");
                return false;
            }

            last_fat_sector = fat_sector;
        }

        // We have reached the end of the updates we can make
        if (updates->entry_number == UINT32_MAX)
                break;

        fat_buffer[fat_offset] = updates->new_value;

        is_first_entry = false;
        updates++;
    }

    return true;
}

fat_directory_entry* s_create_new_dirrectory_entry(uint32_t dirrectory_cluster_number, const char* name, bool is_dirrectory, 
    fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset)
{
    fat_directory_entry entry;
    memclr(&entry, sizeof(fat_directory_entry));
    memcpy(entry.file_name, name, 11);

    if (is_dirrectory)
        entry.attributes = FAT_DIRECTORY_ATTRIBUTES_DIRECTORY;

    fat_entry_update* new_cluster = s_find_free_clusters(1);

    if (new_cluster == NULL)
        return NULL;
    
    bool success = s_write_fat_updates(new_cluster + 1); // + 1 as the first update entry is to set the pervius file end which doesn't exist rn
    uint32_t entry_cluster = new_cluster[1].entry_number;
    free(new_cluster);

    if (!success)      
        return NULL;

    entry.first_cluster_number_higher_16_bits = (entry_cluster >> 16) & 0xFFFF;
    entry.first_cluster_number_lowwer_16_bits = (entry_cluster      ) & 0xFFFF;

    success = s_write_new_dirrectory_entry(dirrectory_cluster_number,   // Cluster number of the dirrectory we are writing into
        &entry,                                                         // The entry to write
        dirrectory_cluster,                                             // Working buffer
        directory_lba, directory_lba_offset);                           // Place to store where the entry is

    if (!success)      
    {
        printf("Warning: Dirrectory entry has not been written but clusters have been allocated!\n");
        return NULL;
    }
    
    // Now if we are writing a dirrectory the . and The .. entrys are nessisary
    if (is_dirrectory)
    {
        memclr(dirrectory_cluster, root_file_system->number_of_sectors_per_cluster * 512);

        memcpy(dirrectory_cluster[0].file_name, "..          " + 1, 11);
        dirrectory_cluster[0].attributes = FAT_DIRECTORY_ATTRIBUTES_DIRECTORY;
        dirrectory_cluster[0].first_cluster_number_higher_16_bits = (entry_cluster >> 16) & 0xFFFF;
        dirrectory_cluster[0].first_cluster_number_lowwer_16_bits = (entry_cluster      ) & 0xFFFF;

        memcpy(dirrectory_cluster[1].file_name, "..          ", 11);
        dirrectory_cluster[1].attributes = FAT_DIRECTORY_ATTRIBUTES_DIRECTORY;
        dirrectory_cluster[1].first_cluster_number_higher_16_bits = (dirrectory_cluster_number >> 16) & 0xFFFF;
        dirrectory_cluster[1].first_cluster_number_lowwer_16_bits = (dirrectory_cluster_number      ) & 0xFFFF;

        uint32_t cluster_lba = root_file_system->data_sector + (entry_cluster - 2) * root_file_system->number_of_sectors_per_cluster;
         
        if (sd_writeblock(cluster_lba, dirrectory_cluster, root_file_system->number_of_sectors_per_cluster) 
            != root_file_system->number_of_sectors_per_cluster * 512) 
        {
            printf("Warning, Failed to initialize new directory!\n");
            return NULL;
        }
    }

    fat_directory_entry* entry_cpy = malloc(sizeof(fat_directory_entry));

    if (entry_cpy == NULL)
    {
        printf("Failed to allocate memory to return copy of new entry, but it does exist on disk!\n");
        return NULL;
    }

    memcpy(entry_cpy, &entry, sizeof(fat_directory_entry));

    return entry_cpy;
}

bool s_write_new_dirrectory_entry(uint32_t dirrectory_cluster_number, const fat_directory_entry* entry, 
    fat_directory_entry* dirrectory_cluster, uint32_t* directory_lba, uint32_t* directory_lba_offset)
{
    uint32_t current_cluster_number = dirrectory_cluster_number;
    uint32_t next_cluster = current_cluster_number;
    bool found_space = false;

    uint32_t cluster_lba = 0;

    while (next_cluster < 0x0FFFFFF8 && found_space == false)
    {   
        current_cluster_number = next_cluster;
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
                return false;
            }
            last_fat_sector = fat_sector;
        }

        next_cluster = fat_buffer[fat_offset] & 0x0FFFFFFF;

        for (int i = 0 ; i < ((512 * root_file_system->number_of_sectors_per_cluster) / sizeof(fat_directory_entry)); i++)
        {
            if (dirrectory_cluster[i].file_name[0] != '\0') // Entry exists (skip ones that dont)
                continue;

            memcpy(dirrectory_cluster + i, entry, sizeof(fat_directory_entry));
            uint32_t entrys_per_sector = 512 / sizeof(fat_directory_entry);
            uint32_t lba = cluster_lba + (i / entrys_per_sector);
            uint32_t offset_entrys = (i % entrys_per_sector);

            if (sd_writeblock(lba, dirrectory_cluster + (i / entrys_per_sector), 1) != 512)
            {
                printf("Erorr: Failed to write to dirrecotry!\n");
                printf("Warning: Dirrectory entry has not been written but clusters have been allocated!\n");
                return false;
            }

            if (directory_lba != NULL)
                *directory_lba = lba;
            if(directory_lba_offset != NULL)
                *directory_lba_offset = offset_entrys % sizeof(fat_directory_entry);

            found_space = true;
            break;
        }
    }

    if (!found_space) // Just make more
    {
        fat_entry_update* new_cluster = s_find_free_clusters(1);

        if (new_cluster == NULL)
            return false;
        
        new_cluster[0].entry_number = current_cluster_number;
        bool success = s_write_fat_updates(new_cluster);
        cluster_lba = root_file_system->data_sector + (new_cluster[1].entry_number - 2) * root_file_system->number_of_sectors_per_cluster;
        free(new_cluster);

        if (!success)
            return false;

        memclr(dirrectory_cluster, root_file_system->number_of_sectors_per_cluster * 512);
        memcpy(dirrectory_cluster, entry, sizeof(fat_directory_entry));

        if (sd_readblock(cluster_lba, dirrectory_cluster, 
        root_file_system->number_of_sectors_per_cluster) 
        != 512 * root_file_system->number_of_sectors_per_cluster)
        {
            printf("Erorr: Failed to write to dirrecotry!\n");
            return false;
        }

        if (directory_lba != NULL)
            *directory_lba = cluster_lba;
        if(directory_lba_offset != NULL)
            *directory_lba_offset = 0;
    }

    return true;
}

file_discriptor_metadata* s_get_file_metadata_from_discriptor(int fd)
{
    fd_hash_table_entry uesd_for_shearch;
    uesd_for_shearch.hash = fd;

    size_t index = dynamic_array_binary_shearch(&s_fd_hash_table, &uesd_for_shearch, s_less_then_fd_table_entry, s_equal_to_fd_table_entry);

    if (index == -1)
        return NULL;

    fd_hash_table_entry* entry = (fd_hash_table_entry*)s_fd_hash_table.ptr;
    entry += index;

    if (s_is_in_user_mode != entry->metadata.is_owened_by_user)
        return NULL;

    return &entry->metadata;
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

void set_user_mode(bool is_in_user_mode)
{
    s_is_in_user_mode = is_in_user_mode;
}