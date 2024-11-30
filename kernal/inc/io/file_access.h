#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct dirrectory_entry
{
    uint64_t size;
    char name[32];
    char extention[8];
};

typedef struct dirrectory_entry dirrectory_entry;


// Initializes the buffesr used to track file discriptors
void initialize_file_access();

// Frees all files assicated with the user
void file_access_on_user_app_exit();

// Used to decide if the kernal or user is using the files
// @param is_in_user_mode True if user, false if kernal
void set_user_mode(bool is_in_user_mode);

// Creates file discriptor for the given file
// @param path Absolute path to file
// @param flags Flags as defined in fcntl.h
// @return -1 On error or file discriptor on success
int open(const char* path, int flags);

// Closes the given file
// @param fs File discriptor to close
// @return -1 On error or 0 on success
int close(int fd);

// Reads n bytes from the given file
// @param fd File discriptor to read from
// @param buf pointer to buffer to read into
// @param n Number of bytes to read
// @return Number of bytes read / 0 if end of file is reached, -1 on Failer
size_t read(int fd, void* buf, size_t n);

// Writes n bytes to the file
// @param fd File discriptor to write to
// @param buf pointer to buffer to write into
// @param n Number of bytes to write
// @return Number of bytes written, -1 on Failer
size_t write(int fd, const void* buf, size_t n);

// Seeks to a offset in the file
// @param fd File discriptor to set the offset of
// @param offset The offset to use
// @param whence Where to seek from
// @return The resulting offset from the start of file, or -1 if failed
ptrdiff_t lseek(int fd, ptrdiff_t offset, int whence);

// Used to set the size of a file to new_size bytes new regions in of the file are zeros
// @param path Path to the file
// @param new_size The new size of the file
// @return 0 if success, -1 if failed
int truncate(const char* path, size_t new_size);

// Used to set the size of a file to new_size bytes new regions in of the file are zeros
// The seek pointer is not changed, unless the old value would be outside the file
// @param fd File discriptor of file to set
// @param new_size The new size of the file
// @return 0 if success, -1 if failed
int ftruncate(int fd, size_t new_size);

// Deletes a file
// @param path Path to the file
// @return 0 if successfull, -1 if Error
int remove(const char* path);

// Deletes a file
// @param fd File discriptor
// @return 0 if successfull, -1 if Error
// @note Closes the file on its own
int fremove(int fd);

// Gets the size of a given file
// @param fd File discriptor of fileFailer
size_t get_file_size(int fd);

// Renames / moves a file
// @param old_path The current path to the file
// @param new_path The new path the file will use
// @return 0 If success, non-zero on failer
int rename(const char* old_path, const char* new_path);

// Checks if a file exists
// @param path Path to file / dirrectory to check existance of
// @return 1 if exists 0 if not -1 on failer
int path_exists(const char* path);

// Used to list the contents of a dirrectory
// 1. Open 
// 2. Read untill it returns <= 0
// 3. Close
// Simuler to the linux opendir, but the struct is diffrent so diffrent name
// @param path Path of dirrectory to check
// @return A dirrectory discriptor, or -1 if failed
int diropen(const char* path);

// Reads a dirrectory and returns one entry as a dirrectory_entry struct
// @param dd Dirrectory discriptor of dirrectory to read
// @param entry Pointer to dirrectory_entry struct to fill out
// @return 1 Means success, 0 means end of dirrectory, > 0 means error
int dirread(int dd, dirrectory_entry* entry);

// Closes the given dirrectory
// @param dd Dirrectory discriptor  to close
// @return 0 on success, non-zero on failer
int dirclose(int dd);


enum
{
    SEEK_SET    =   0,  /* set file offset to offset */
    SEEK_CUR    =	1,  /* set file offset to current plus offset */
    SEEK_END    =	2   /* set file offset to EOF plus offset */
};

enum
{
    FILE_FLAGS_READ         = 0,            /* As the name states read only*/
    FILE_FLAGS_READ_WRITE   = 1 << 0,       /* As the name states read / write*/
    FILE_FLAGS_APPEND       = 1 << 1,       /* Seek to the end of file when opened*/
    FILE_FLAGS_CREATE       = 1 << 2,       /* If file not found, create a new one (Must have FILE_FLAGS_READ_WRITE)*/
    FILE_FLAGS_TRUNCATE     = 1 << 3,       /* If file allready exists and is writeable the file is truncated to zero bytes on opening*/

    FILE_FLAGS_UNUSED_BITS = UINT32_MAX & ~(FILE_FLAGS_READ_WRITE | FILE_FLAGS_APPEND | FILE_FLAGS_CREATE | FILE_FLAGS_TRUNCATE),
};

#endif