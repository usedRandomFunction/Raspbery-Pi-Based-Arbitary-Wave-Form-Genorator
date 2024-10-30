#include "lib/user_program.h"
#include "lib/exceptions.h"
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
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", path, "open");

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
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", buf, "read");

    set_user_mode(true);
    size_t r = read(fd, buf, n);
    set_user_mode(false);

    return r;
}

size_t system_call_write(int fd, const void* buf, size_t n)
{
    if (is_kernal_memory(buf))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", buf, "write");
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
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", path, "truncate");

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
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", path, "remove");

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

void system_call_undefined_handler()
{
    size_t w8;

    asm volatile ( "mov %0, x8" : "=r"(w8));

    w8 &= 0xFFFFFFFF; // I cant load 32 bits dirrectly so load 64 and bitwise and it

    generic_user_exception("User attempted to call undefined system call %d!\n", w8);
}

size_t system_call_vmemmap(void* ptr, size_t size, int flags)
{
    return user_program_vmemmap(get_active_user_program(),
        ptr, size, flags);
}


void* const system_call_table[] = {system_call_set_abi_version, // ABI commands
    system_call_exit,                                           // Proccess controll commands
    printf_user_memory_only, putchar,                           // Print commands
    system_call_uart_putc, uart_getc, uart_poll,                // UART commands
    system_call_open, system_call_close,                        // File IO
    system_call_get_file_size, system_call_read,                // More File IO
    system_call_write, system_call_lseek, system_call_truncate, // Even more FILE IO
    system_call_ftruncate, system_call_remove,                  // Yup File IO
    system_call_fremove, system_call_undefined_handler,         // Ok, its done
    system_call_vmemmap                                         //
};