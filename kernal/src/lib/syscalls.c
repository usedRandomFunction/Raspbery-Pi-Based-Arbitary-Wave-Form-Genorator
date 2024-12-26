#include "lib/user_program.h"
#include "lib/exceptions.h"
#include "lib/interrupts.h"
#include "io/file_access.h"
#include "io/data_output.h"
#include "lib/memory.h"
#include "io/putchar.h"
#include "io/printf.h"
#include "io/keypad.h"
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

int system_call_rename(const char* old_path, const char* new_path)
{
    if (is_kernal_memory(old_path))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", old_path, "rename");

    if (is_kernal_memory(new_path))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", new_path, "rename");

    set_user_mode(true);
    int r = rename(old_path, new_path);
    set_user_mode(false);

    return r;
}

int system_call_path_exists(const char* path)
{
    if (is_kernal_memory(path))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", path, "path_exists");

    set_user_mode(true);
    int r = path_exists(path);
    set_user_mode(false);

    return r;
}

int system_call_diropen(const char* path)
{
    if (is_kernal_memory(path))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", path, "diropen");

    set_user_mode(true);
    int r = diropen(path);
    set_user_mode(false);

    return r;
}

int system_call_dirread(int dd, dirrectory_entry* entry)
{
    if (is_kernal_memory(entry))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", entry, "dirread");

    set_user_mode(true);
    int r = dirread(dd, entry);
    set_user_mode(false);

    return r;
}

int system_call_dirclose(int dd)
{
    set_user_mode(true);
    int r = dirclose(dd);
    set_user_mode(false);

    return r;
}

void system_call_undefined_handler()
{
    size_t esr_el1;
    size_t w8;

    asm volatile ( "mov %0, x8" : "=r"(w8));
    asm volatile ( "mrs %0, esr_el1" : "=r"(esr_el1));

    esr_el1 &= 0xFFFF;      // Get just the svc argument
    w8 &= 0xFFFFFFFF;       // I cant load 32 bits dirrectly so load 64 and bitwise and it
    w8 /= 8;                // w8 has been timesed by 8 before so we need to undo it

    generic_user_exception("User attempted to call undefined system call: SVC #0x%x, w8=%d!\n", esr_el1, w8);
}

size_t system_call_vmemmap(void* ptr, size_t size, int flags)
{
    return user_program_vmemmap(get_active_user_program(),
        ptr, size, flags);
}

void system_call_switch_to(const char* new_executable_path)
{
    if (is_kernal_memory(new_executable_path))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", new_executable_path, "switch_to");

    user_program_switch_to(new_executable_path);
}

void _trigger_user_prg_exit_handler()
{
    trigger_user_interupt_handler(0);
}

void system_call_capture_prg_exit(void* handler)
{
    if (is_kernal_memory(handler))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", handler, "capture_prg_exit");

    if (handler == NULL)                // Handle dissabling the function
    {
        handler = (void*)POINTER_MAX;
        capture_prg_exit(NULL);

        return;
    }

    register_user_interupt_handler(handler, 0);

    capture_prg_exit(_trigger_user_prg_exit_handler);
}

int system_call_dac_output_start(void* buffer_start, size_t n, int flags)
{
    if (is_kernal_memory(buffer_start))
        generic_user_exception("User attempted to access kernal memory address: 0x%x, when calling %s\n", buffer_start, "dac_output_start");

    size_t spsr_el1;
    size_t elr_el1;
    asm volatile ( "mrs %0, spsr_el1" : "=r"(spsr_el1));    // Save these so they dont get corrupted
    asm volatile ( "mrs %0, elr_el1" : "=r"(elr_el1));

    asm volatile ("msr daifclr, #2 ");  // Enable IRQs since dac_output_start is designed to be exited with system calls

    int return_value = dac_output_start(buffer_start, n, flags);

    asm volatile ("msr daifset, #2 ");  // Dissable IRQs so interupts to mess up syscalls in the future.

    asm volatile ( "msr spsr_el1, %0" : : "r"(spsr_el1));    // Reset them
    asm volatile ( "msr elr_el1, %0" : : "r"(elr_el1));

    return return_value;
}

const void* const os_syscall_program_managment[] = {system_call_set_abi_version, system_call_exit, system_call_vmemmap,
    system_call_switch_to};
const void* const os_syscall_baisc_io[] = {printf_user_memory_only, putchar, uart_putc, uart_getc, uart_poll};
const void* const os_syscall_file_io[] = {system_call_open, system_call_close, system_call_get_file_size, system_call_read, 
    system_call_write, system_call_lseek, system_call_truncate, system_call_ftruncate, system_call_remove,   
    system_call_fremove, system_call_rename, system_call_path_exists, system_call_diropen, system_call_dirread,
    system_call_dirclose};

const void* const os_syscall_tables_table[] = {os_syscall_program_managment, os_syscall_baisc_io, os_syscall_file_io};
const uint64_t os_syscall_tables_sizes_table[] = {sizeof(os_syscall_program_managment), 
    sizeof(os_syscall_baisc_io), sizeof(os_syscall_file_io)};
const uint64_t size_of_os_syscall_tables_sizes_table = sizeof(os_syscall_tables_sizes_table);


const void* const hardware_outputs[] = {system_call_dac_output_start, dac_output_end, dac_resolution, dac_channel_buffering,
    dac_get_sample_rate, dac_channel_supports_config};
const void* const project_specfic_keypad[] = {keypad_polling, uart_keypad_emmulation, system_call_capture_prg_exit, get_keypad_state};

const void* const project_specfic_syscall_tables_table[] = {hardware_outputs, project_specfic_keypad};
const uint64_t project_specfic_syscall_tables_sizes_table[] = {sizeof(hardware_outputs), sizeof(project_specfic_keypad)};
const uint64_t size_of_project_specfic_syscall_tables_sizes_table = sizeof(project_specfic_syscall_tables_sizes_table);