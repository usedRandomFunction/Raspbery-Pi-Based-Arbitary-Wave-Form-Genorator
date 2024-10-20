#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include <stdint.h>
#include <stddef.h>


// Initializes the buffesr used to track file discriptors
void initialize_file_access();

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

// Gets the size of a given file
// @param fd File discriptor of file
// @return size of file in bytes / -1 on Failer
size_t get_file_size(int fd);

enum
{
    SEEK_SET    =   0,  /* set file offset to offset */
    SEEK_CUR    =	1,  /* set file offset to current plus offset */
    SEEK_END    =	2   /* set file offset to EOF plus offset */
};

enum
{
    FILE_FLAGS_READ         = 0,
    FILE_FLAGS_READ_WRITE   = 1 << 0,

    FILE_FLAGS_UNUSED_BITS = UINT32_MAX & ~(FILE_FLAGS_READ_WRITE),
};

#endif