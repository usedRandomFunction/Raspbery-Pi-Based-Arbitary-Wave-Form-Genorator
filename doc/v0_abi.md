# V0 ABI

The V0 ABI is designed to be expanedable so if the OS is used in a embeded projectwhere hardware functions are abstracted. <br>
Because of this svc #0x0 to svc #0x7FFF are reseverd for the os while #0xFF00 to #0xFFFF are for the user

A syscall is made by setting w8 then calling svc

note that only OS functions are included here not project specific syscalls <br>
If any they should be [listed here](./project_specific_syscalls.md)


## syscall tables vs SVC argument

## #0x00 - Program managment

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | set_abi_version | int version | int    | Sets the ABI to the given version as the name implies, reutrns -1 if failed <br> and zero on success
| 1 | exit  | int status | N/A | Exits the current program and gives a status number
| 2 | vmemmap | void* ptr, size_t size<br>int [flags](#vmemmap-flags) | size_t | Creates / deletes / modifys, virtual memory mappings. <br> It attempts to put the mapping at ptr in memory and will fail if it cant.<br>When a entry at ptr doesn't exist it will create on, if it exists it will modify <br>/ delete it. <br>If size > 0, it will create / modify, and will return the size of the allocation in <br>bytes or 0 if it failed.<br>If size = 0, it will attempt to delete it returning > 0 on success and 0 on failer<br>If size = -1, it will return the current size of the allocation<br><br>Note that attempting to delete a allocation doesn't exist will fail. <br> also [see flags](#vmemmap-flags)
| 3 | switch_to | const char* <br> new_executable_path| N/A | Exits the current program and then starts the given program

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
| 3 | uart_getc | void | char | Gets a charector from the UART and will wait for one to be avalible
| 4 | uart_poll | void | int | Same as uart_getc but doesn't wait, and will return 0XFFFF, if nothing is available

## #0x02 - File IO

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | open | const char* path,<br>int [flags](#open-flags) | int | Opens the given file if possible and returns file descriptor <br> or -1 if failed. [See flags](#open-flags)
| 1 | close | int fd | int | Closes the given file, reutrns 0 on success and -1 on failer
| 2 | get_file_size | int fd | size_t | Takes the given file and returns is size in bytes or -1 on failer
| 3 | read | int fd, void* buf, <br>size_t n | size_t | Reads n bytes from the file, returns number of bytes read or 0 on <br> reaching end of file gives -1 on error
| 4 | write | int fd, const void* buf, <br>size_t n | size_t | Writes n bytes to the file, returns number of bytes written, <br>gives -1 on error
| 5 | lseek | int fd ptrdiff_t offset,<br>int [whence](#seek-whence) | ptrdiff_t | Seeks to a offset in the file, also read [whence](#seek-whence)
| 6 | truncate | const char* path, <br> size_t new_size | int | Sets the size of the file n bytes, returns zero on success, -1 on failer
| 7 | ftruncate | itn fd, size_t new_size | int | Sets the size of the file n bytes, returns zero on success, -1 on failer
| 8 | remove | const char* path | int | Deletes a file, returns 0 on success, -1 on failer
| 9 | fremove | int fd | int | Deletes a file, returns 0 on success, -1 on failer
| 10 | rename | const char* old_path, const char* new_path | int | Renames a file / moves it to a new dirrectory. Returns 0 on success, <br> Non-zero of failure
| 11 | path_exists | const char* path | int | Checks if a file / dirrectory exists. Returns 1 if existance, 0 if not,<br> -1 on failer
| 12 | diropen | const char* path | int | Opens the given dirrectory if possible and returns dirrectory descriptor <br> or -1 if failed.
| 13 | dirread | int dd, dirrectory_entry* entry | int | Returns one entry from the given dirrectory. Returns 1 On success, <br> 0 if no more entrys exist and < 0 on error. [See dirrectory_entry struct](#dirrectory_entry-struct)
| 14 | dirclose | int dd | int | Closes the given dirrectory, Returns zero on success non-zero on failer

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

#### dirrectory_entry struct

The dirrectory_entry struct is defined simply as:

```c
struct dirrectory_entry
{
    uint64_t size;
    char name[32];
    char extention[8];
};
```

## #0x03 - Display controll

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | set_display_pixel | uint32_t x, uint32_t y,<br>display_color color,<br> int buffer | void | Sets the color of the pixel at (x, y) to `color`.<br>`buffer` is the ID of the frame buffer to write to,<br> If -1 is given, the active frame buffer will be used.
| 1 | display_fill_rect | uint32_t x1, uint32_t x2,<br>uint32_t y1, uint32_t y2,<br>display_color color,<br>int buffer | void | Fills a rectangle from (x1, y1) [top left] to<br>(x2, y2) bottom right, setting the color to `color`.<br>`buffer` is the ID of the frame buffer to write to,<br>If -1 is given, the active frame buffer will be used.
| 2 | copy_to_display | uint32_t x, uint32_t size_x,<br>uint32_t y, uint32_t size_y,<br> display_color* data, <br> int pixels_per_line, int buffer | void | Copys [size_x, size_y] pixels from `data` to the<br> screen with (x, y) as the top left corner. `pixels_per_line`<br> stores the number of pixels in a line for the `source` buffer.<br>if `0` If given it will defult to size_x<br>`buffer` is the ID of the frame buffer to write to,<br>If -1 is given, the active frame buffer will be used.
| 3 | display_draw_string | const char* str, uint32_t* x, uint32_t* y<br> uint32_t x_min, uint32_t x_max,<br>bool are_special_characters_enabled,<br>pc_screen_font_header* font,<br> display_color foreground,<br>display_color background<br>int buffer | void | Draws the given (null terminated) string starting at (x, y), [top left].<br>Word wraping is enabled using `x_max` and `x_min`.<br>If `are_special_characters_enabled` is set to true characters like<br>'\n' or '\r' work, if set to false they are ignored. The font is configurable<br>using `font`, if null is given the kernel defult is used. The foreground<br>and background colors are also configurable.<br>`buffer` is the ID of the frame buffer to write to,<br>If -1 is given, the active frame buffer will be used.
| 4 | get_display_width | void | uint32_t | Returns the width of the display in pixels
| 5 | get_display_height | void | uint32_t  | Returns the height of the display in pixels
| 6 | active_frame_buffer | int buffer | int | Gets / set the active frame buffer, if `-1` is given no action will be taken.<br>`-2` is the printf / stdout buffer, user programs cant edit it dirrectly.<br>`0` up to `nbuffers - 1` is available to set to and these buffers are editable.<br><br>The return value is simply the active output buffer at the end of the operation.
| 7 | request_frame_buffers | int nbuffers | int | Gets / set the number of frmae buffers allocated to the user program.<br>Like other functions if `-1` is given no action will be taken, and the current<br>value will be returned.<br><br>Return the number of frame buffers available to the user program at the <br>end of the operation.

#### Note about frame buffers

While the raspberry pi only has one frame buffer, the kernel uses a singal<br>
large buffer with virtual offsets to emulaute multiple frame buffers.<br>
Also note, frame buffer `-2` does exist and can be switched to, however<br>
however it can not be written to, as it is used by the kernel to display,<br>
`putchar` and `printf`.

#### display_color

`display_color` is defined as
```c
typedef uint32_t display_color;
```

Starting from the left most bit, it is stored as red, blue, green, alpha,<br>
all the values are 8 bit. For alpha 0 is opaque, and 255 is completely transparent.<br>
Note that alpha is only used when writing to the frame buffer and is not stored. 

#### pc_screen_font_header struct

```c
struct pc_screen_font_header
{
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
    uint8_t glyphs;
};
```

This struct is simply the header of a `PSF2` file.<br>
Note that the glyphs must start immediately after the struct in memory.

# 0x04 - Runtime Editing of config settings

## Note

These functions <i><b>do not</b></i> edit the underlying config files e.g `system.cfg`.<br>
They only edit the values stored in ram, and are to be used to test new values

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | Reserved | N/A | N/A | N/A |
| 1 | set_display_overscan | uint32_t top, uint32_t bottom,<br>uint32_t left, uint32_t right | void | As the name says, sets the display overscan