# Applications

as of the current time only "monolithic" programs are supported
this means that all the data and code is stored in one seciton

## Application basic interface (ABI)

TO BE FILLED OUT WHEN THE V0 ABI IS FINALISED

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
| `PROGRAM_MEMORY_WRITABILITY`  | When set to zero (which is the default state if not pressent) The main page will be set to read only by default.
| `MONOLITHIC_PAGE_SIZE`        | This tells the kernal how big to make the main page, if this value is not pressent the kernal uses the size of <br>the image file. If the image file is larger then this value it will be cut off, if this value is larger then <br>the size the extra bytes are set to zero.
| `MINIUM_STACK_SIZE`           | This value tells the kernal will guarantee the stack to be at lesat this many bytes. <br> This entry defaults to 1024 bytes is unset
| `PROGRAM_ENTRY`               | This value tells the kernal where the entry function is, if it is not pressent PROGRAM_ADDRESS is used.
| `ABI_VERSION`                 | This tells the kernal what version of the ABI the program is expecting, and defaults to zero
