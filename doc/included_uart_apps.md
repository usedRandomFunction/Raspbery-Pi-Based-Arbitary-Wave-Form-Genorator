# Included uart apps

Currently there is only one included uart app [`uartrun`](#uartrun).

## uartrun

Uart run allows a another app to be uploaded and run without writting to the disk.<br>
As a result it has two main diffrences to reguler running.

1. The loaders .text, and .bss (If it exists) are still loaded
2. The stack size is fixed and therefore will be less then 16 KiB

### Communication protocall

Messages are sent as a base packet header then a type specific header. <br>
For some types this is all that needs to be  done but for others further, <br>
messages are required.

Before sending messages the user has to wait for the magic word:<br>
`55 41 52 54 52 44 59 0A`  "UARTRDY\n"<br>

Packet base and type specific headers are sent as one, while further<br>
messages (including if its a part of the same packet) need to wait for<br>
another magic word. 


Note all values in the packet headers are little edin

#### Basic header

| Offset<br>(bytes) | Size<br>(bytes)| Name | Description |
|-------------------|----------------|------|-------------|
| 0x00 | 0x01 | type | The packet type 

#### Packet Types

| Type | Packet |
|------|--------|
| 0 | [Ignore packet](#ignore-packet)
| 1 | [Execute packet](#execute-packet)
| 2 | [Section packet](#section-packet)
| 3 | [Memset packet](#memset-packet)
| 4 | [Memcpy packet](#memcpy-packet)

#### Ignore packet

This is a very simple packet, it is ignored.<br>
There is non type specific header

#### Execute packet

This packet will simplally jump to the loaded program.

| Offset<br>(bytes) | Size<br>(bytes)| Name | Description |
|-------------------|----------------|------|-------------|
| 0x00 | 0x08 | entry | Pointer to entry of the program

#### Section packet

This is used to indicate the size, possition and flags of section in memory<br>
Its header is as follows:

| Offset<br>(bytes) | Size<br>(bytes)| Name | Description |
|-------------------|----------------|------|-------------|
| 0x00 | 0x08 | section_beign | The location in memory this section will take
| 0x08 | 0x08 | section_size | The minium size of the section in bytes
| 0x10 | 0x04 | flags | Section flags (As defined for [vmemmap](./v0_abi.md#vmemmap-flags))

#### Memset packet

Sets `size` bytes from `start` to `value`

| Offset<br>(bytes) | Size<br>(bytes)| Name | Description |
|-------------------|----------------|------|-------------|
| 0x00 | 0x08 | start | Where to start the memset opperation
| 0x08 | 0x08 | size  | The number of bytes to set
| 0x10 | 0x01 | value | The value to set these bytes to

#### Memcpy packet

Recives `size` bytes from the uart and writes then to `start`

| Offset<br>(bytes) | Size<br>(bytes)| Name | Description |
|-------------------|----------------|------|-------------|
| 0x00 | 0x08 | start | Where to start storing the bytes
| 0x08 | 0x08 | size  | The number of bytes to recive

After sending the packet header, the user needs to wait for the next magic word, <br>
befor sending the data over.