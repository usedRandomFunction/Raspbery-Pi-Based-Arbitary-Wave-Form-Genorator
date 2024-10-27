#include "lib/arm_exceptions.h"
#include "io/file_access.h"
#include "lib/memory.h"
#include "io/putchar.h"
#include "io/printf.h"
#include "io/uart.h"

extern void system_call_exit(int i);

int system_call_uart_putc(char c)
{
    uart_putc(c);

    return 0;   // So r0 Doesn't give anything away
}

int system_call_set_abi_version(int version)
{
    if (version != 0)
        return -1;

    return 0;
}

int system_call_open(const char* path, int flags)
{   
    if (is_kernal_memory(path))
        kernel_panic(); // We do this for now untill there is a way to sepaote user and kernal errors k?

    set_user_mode(true);
    int r = open(path, flags);
    set_user_mode(false);

    return r;
}

int system_call_close(int fd)
{
    set_user_mode(true);
    int r = close(fd);
    set_user_mode(false);

    return r;
}

size_t system_call_get_file_size(int fd)
{
    set_user_mode(true);
    size_t r = get_file_size(fd);
    set_user_mode(false);

    return r;
}

size_t system_call_read(int fd, void* buf, size_t n)
{
    if (is_kernal_memory(buf))
        kernel_panic(); // We do this for now untill there is a way to sepaote user and kernal errors k?

    set_user_mode(true);
    size_t r = read(fd, buf, n);
    set_user_mode(false);

    return r;
}

size_t system_call_write(int fd, const void* buf, size_t n)
{
    if (is_kernal_memory(buf))
        kernel_panic(); // We do this for now untill there is a way to sepaote user and kernal errors k?

    set_user_mode(true);
    size_t r = write(fd, buf, n);
    set_user_mode(false);

    return r;
}

ptrdiff_t system_call_lseek(int fd, ptrdiff_t offset, int whence)
{
    set_user_mode(true);
    ptrdiff_t r = lseek(fd, offset, whence);
    set_user_mode(false);

    return r;
}

int system_call_truncate(const char* path, size_t new_size)
{
    if (is_kernal_memory(path))
        kernel_panic(); // We do this for now untill there is a way to sepaote user and kernal errors k?

    set_user_mode(true);
    int r = truncate(path, new_size);
    set_user_mode(false);

    return r;
}

int system_call_ftruncate(int fd, size_t new_size)
{
    set_user_mode(true);
    int r = ftruncate(fd, new_size);
    set_user_mode(false);

    return r;
}

int system_call_remove(const char* path)
{
    if (is_kernal_memory(path))
        kernel_panic(); // We do this for now untill there is a way to sepaote user and kernal errors k?

    set_user_mode(true);
    int r = remove(path);
    set_user_mode(false);

    return r;
}

int system_call_fremove(int fd)
{
    set_user_mode(true);
    int r = fremove(fd);
    set_user_mode(false);

    return r;
}

void* const system_call_table[] = {system_call_set_abi_version, // ABI commands
    system_call_exit,                                           // Proccess controll commands
    printf_user_memory_only, putchar,                           // Print commands
    system_call_uart_putc, uart_getc, uart_poll,                // UART commands
    system_call_open, system_call_close,                        // File IO
    system_call_get_file_size, system_call_read,                // More File IO
    system_call_write, system_call_lseek, system_call_truncate, // Even more FILE IO
    system_call_ftruncate, system_call_remove,                  // Yup File IO
    system_call_fremove                                         // Ok, its done
};