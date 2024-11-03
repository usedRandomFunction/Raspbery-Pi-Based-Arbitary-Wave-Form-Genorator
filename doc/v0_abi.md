# V0 ABI

The V0 ABI is designed to be expanedable so if the OS is used in a embeded projectwhere hardware functions are abstracted. <br>
Because of this svc #0x0 to svc #0xFF are reseverd for the os while #0xFF00 to #0xFFFF are for the user

A syscall is made by setting w8 then calling svc

note that only OS functions are included here not project specific syscalls <br>
If any they should be [listed here](./project_specific_syscalls)


## syscall tables vs SVC argument

## #0x00 - Program managment

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | set_abi_version | int version | int    | Sets the ABI to the given version as the name implies, reutrns -1 if failed <br> and zero on success
| 1 | exit  | int status | N/A | Exits the current program and gives a status number
| 2 | vmemmap | void* ptr, size_t size<br>int [flags](#vmemmap-flags) | size_t | Creates / deletes / modifys, virtual memory mappings. <br> It attempts to put the mapping at ptr in memory and will fail if it cant.<br>When a entry at ptr doesn't exist it will create on, if it exists it will modify <br>/ delete it. <br>If size > 0, it will create / modify, and will return the size of the allocation in <br>bytes or 0 if it failed.<br>If size = 0, it will attempt to delete it returning > 0 on success and 0 on failer<br>Note that attempting to delete a allocation doesn't exist will fail. <br> also [see flags](#vmemmap-flags)

#### vmemmap flags

| Value | Name | Description |
|-------|------|-------------|
| 1 | VMEMMAP_WRITABILITY | If set the memory will be writeable
| 2 | VMEMMAP_NON_CACHABLE | If set the memory will be non-cachable
| 4 | VMEMMAP_EXECUTABLE | If set the memory will be executable 

## #0x01 - Basic IO

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | printf | const char* str, ... | void | Simple printf function, prints to UART and screen
| 1 | putchar | char c | int | Puts a charector on to the UART and screen, returns EOF on failed
| 2 | uart_putc | char c | void | Puts a charector onto the UART
| 3 | uart_getc | N/A | char | Gets a charector from the UART and will wait for one to be avalible
| 4 | uart_poll | N/A | int | Same as uart_getc but doesn't wait, and will return 0XFFFF, if nothing is avaible

## #0x02 - File IO

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | open | const char* path,<br>int [flags](#open-flags) | int | Opens the given file if possible and returns file descriptor <br> or -1 if failed. [See flags](#open-flags)
| 1 | close | int fd | int | Closes the given file, reutrns 0 on success and -1 on failer
| 2 | get_file_size | int fd | size_t | Takes the given file and returns is size in bytes or -1 on failer
| 3 | read | int fd, void* buf, <br>size_t n | size_t | Reads n bytes from the file, returns number of bytes read or 0 on <br> reaching end of file gives -1 on error
| 4 | write | int fd, const void* buf, <br>size_t n | size_t | Writes n bytes to the file, returns number of bytes written, gives -1 on error
| 5 | lseek | int fd ptrdiff_t offset,<br>int [whence](#seek-whence) | ptrdiff_t | Seeks to a offset in the file, also read [whence](#seek-whence)
| 6 | truncate | const char* path, <br> size_t new_size | int | Sets the size of the file n bytes, returns zero on success, -1 on failer
| 7 | ftruncate | itn fd, size_t new_size | int | Sets the size of the file n bytes, returns zero on success, -1 on failer
| 8 | remove | const char* path | int | Deletes a file, returns 0 on success, -1 on failer
| 9 | fremove | int fd | int | Deletes a file, returns 0 on success, -1 on failer

#### open flags
The flags used by the open function are as follows

| Value | Name | Description |
|-------|------|-------------|
| 0 | FILE_FLAGS_READ | Sets the file to read only
| 1 | FILE_FLAGS_READ_WRITE | Sets the file to be read / write
| 2 | FILE_FLAGS_APPEND | If it exists, will seek to the end of the file on opening
| 4 | FILE_FLAGS_CREATE | If not found will attempt to create the file
| 8 | FILE_FLAGS_TRUNCATE | If the file allready exists will set the size to zero

#### seek whence
The flags used by the seek function are as follows

| Value | Name | Description |
|-------|------|-------------|
| 0 | SEEK_SET | Sets the offset of the file to the given offest
| 1 | SEEK_CUR | Adds the given offset to the current offest
| 2 | SEEK_END | Sets the offset relitive to the end of the file