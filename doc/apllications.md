# Applications

as of the current time only "monolithic" programs are supported
this means that all the data and code is stored in one seciton

## Application basic interface (ABI)

There is currently one ABI availible to user apps "v0"

in this the calls are made by setting w8 to a number and then calling svc #0

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | set_abi_version | int version | int    | Sets the ABI to the given version as the name implies, reutrns -1 if failed <br> and zero on success
| 1 | exit  | int status | N/A | Exits the current program and gives a status number
| 2 | printf | const char* str, ... | void | Simple printf function, prints to UART and screen
| 3 | putchar | char c | int | Puts a charector on to the UART and screen, returns EOF on failed
| 4 | uart_putc | char c | void | Puts a charector onto the UART
| 5 | uart_getc | N/A | char | Gets a charector from the UART and will wait for one to be avalible
| 6 | uart_poll | N/A | int | Same as uart_getc but doesn't wait, and will return 0XFFFF, if nothing is avaible
| 7 | open | const char* path,<br>int [flags](#open-flags) | int | Opens the given file if possible and returns file descriptor <br> or -1 if failed. See [flags](#open-flags)
| 8 | close | int fd | int | Closes the given file, reutrns 0 on success and -1 on failer
| 9 | get_file_size | int fd | size_t | Takes the given file and returns is size in bytes or -1 on failer
| 10 | read | int fd, void* buf, <br>size_t n | size_t | Reads n bytes from the file, returns number of bytes read or 0 on <br> reaching end of file gives -1 on error
| 11 | write | int fd, const void* buf, <br>size_t n | size_t | Writes n bytes to the file, returns number of bytes written, gives -1 on error
| 12 | lseek | int fd ptrdiff_t offset,<br>int [whence](#seek-whence) | ptrdiff_t | Seeks to a offset in the file, also read [whence](#seek-whence)
| 13 | truncate | const char* path, <br> size_t new_size | int | Sets the size of the file n bytes, returns zero on success, -1 on failer
| 14 | ftruncate | itn fd, size_t new_size | int | Sets the size of the file n bytes, returns zero on success, -1 on failer
| 15 | remove | const char* path | int | Deletes a file, returns 0 on success, -1 on failer
| 16 | fremove | int fd | int | Deletes a file, returns 0 on success, -1 on failer
| 17 | RESRVED | N/A | N/A | reserved for file rename function
| 18 | vmemmap | void* ptr, size_t size<br>int [flags](#vmemmap-flags) | size_t | Creates / deletes / modifys, virtual memory mappings. <br> It attempts to put the mapping at ptr in memory and will fail if it cant.<br>When a entry at ptr doesn't exist it will create on, if it exists it will modify <br>/ delete it. <br>If size > 0, it will create / modify, and will return the size of the allocation in <br>bytes or 0 if it failed.<br>If size = 0, it will attempt to delete it returning > 0 on success and 0 on failer<br>Note that attempting to delete a allocation doesn't exist will fail. <br> also see [flags](#vmemmap-flags)


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

### vmemmap flags

| Value | Name | Description |
|-------|------|-------------|
| 1 | VMEMMAP_WRITABILITY | If set the memory will be writeable
| 2 | VMEMMAP_NON_CACHABLE | If set the memory will be non-cachable
| 4 | VMEMMAP_EXECUTABLE | If set the memory will be executable 

## Application as a configuration file

the smallest config file for a application is as follows

``` ini
APPLICATION_TYPE = MONOLITHIC
PROGRAM_ADDRESS = 0xADDRESS
IMAGE_PATH = "IMAGE_FILE_PATH"
```

This contains all mandatory entrys and they are as follows:

|                    | |
|--------------------|-|
| `APPLICATION_TYPE` | This tells the kernal what type of application this is currently only `MONOLITHIC` is supported
| `PROGRAM_ADDRESS`  | This tells the kernal where to store the program into this must be 1 GiB alligned and inside user space
| `IMAGE_PATH`       | This tells the kernal where to load the application from, *Note that this is relitive to the current config file*


However there are five more optinal entrys:

|                               | |
|-------------------------------|-|
| `PROGRAM_MEMORY_WRITABILITY`  | When set to zero the main page will be set to read only and this is the defualt behaviour
| `MONOLITHIC_PAGE_SIZE`        | This tells the kernal how big to make the main page, if this value is not pressent the kernal uses the size of <br>the image file. If the image file is larger then this value it will be cut off, if this value is larger then <br>the size the extra bytes are set to zero.
| `MINIUM_STACK_SIZE`           | This value tells the kernal will guarantee the stack to be at lesat this many bytes. <br> This entry defaults to 1024 bytes is unset
| `PROGRAM_ENTRY`               | This value tells the kernal where the entry function is, if it is not pressent PROGRAM_ADDRESS is used.
| `ABI_VERSION`                 | This tells the kernal what version of the ABI the program is expecting, and defaults to zero
