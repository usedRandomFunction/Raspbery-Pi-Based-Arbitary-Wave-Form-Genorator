# Applications

as of the current time only "monolithic" programs are supported
this means that all the data and code is stored in one seciton

## Application basic interface (ABI)

There is currently one ABI availible to user apps "v0"<br>
Documention for "v0" can be [found here](./v0_abi.md)

## Application as a configuration file

the smallest config file for a application is as follows

``` ini
APPLICATION_TYPE = MONOLITHIC
PROGRAM_ADDRESS = 0xADDRESS
IMAGE_PATH = "IMAGE_FILE_PATH"
```

This contains all mandatory entrys and they are as follows:

|        Name        | Description |
|--------------------|-|
| `APPLICATION_TYPE` | This tells the kernal what type of application this is currently only `MONOLITHIC` is supported
| `PROGRAM_ADDRESS`  | This tells the kernal where to store the program into this must be 1 GiB alligned and inside user space
| `IMAGE_PATH`       | This tells the kernal where to load the application from, *Note that this is relitive to the current config file*


However there are five more optinal entrys:

| Name | Description | Defult Value |
|------|-------------|--------------|
| `PROGRAM_MEMORY_WRITABILITY`  | When set to zero the main page will be set to read only all other values allow writes | 0
| `MONOLITHIC_PAGE_SIZE`        | This tells the kernal how big to make the main page, If the image file is larger then this <br> value it will be cut off, if this value is larger then the size the extra bytes are set to zero. | Size of the image file
| `MINIUM_STACK_SIZE`           | This value tells the kernal will guarantee the stack to be at lesat this many bytes. | 1024
| `PROGRAM_ENTRY`               | This value tells the kernal where the entry function is | PROGRAM_ADDRESS
| `STACK_ADDRESS`               | This value tells the kernal where to put the stack in memory. This is the lowest address <br> in memory | 128 Gib above <br> PROGRAM_ADDRESS
| `ABI_VERSION`                 | This tells the kernal what version of the ABI the program is expecting | 0