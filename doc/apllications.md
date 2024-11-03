# Applications

as of the current time only "monolithic" programs are supported
this means that all the data and code is stored in one seciton

## Application basic interface (ABI)

There is currently one ABI availible to user apps "v0"<br>
Documention for "v0" can be [found here](./v0_abi.md)


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
