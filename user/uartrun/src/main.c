#include "common/program_managment.h"
#include "common/basic_io.h"
#include "common/memory.h"
#include "common/keypad.h"
#include "common/uart.h"

#include <stdint.h>

struct uart_packet_header
{
    uint8_t type;

    uint8_t data[255];  // Used to store the specfic header
}__attribute__((packed));

typedef struct uart_packet_header uart_packet_header;

struct uart_execute_packet_header
{
    void* entry;
}__attribute__((packed));

typedef struct uart_execute_packet_header uart_execute_packet_header;

struct uart_section_packet_header
{
    void* section_beign;        // Pointer to where it begins
    uint64_t section_size;      // Size of section in bytes
    int flags;                  // Flags sent to vmemmap
}__attribute__((packed));

typedef struct uart_section_packet_header uart_section_packet_header;

struct uart_memset_packet_header
{
    void* start;
    uint64_t size;
    uint8_t value;
}__attribute__((packed));

typedef struct uart_memset_packet_header uart_memset_packet_header;

struct uart_memcpy_packet_header
{
    uint8_t* start;
    uint64_t size;
}__attribute__((packed));

typedef struct uart_memcpy_packet_header uart_memcpy_packet_header;


const char ready_magic_number[] = "UARTRDY\n";

enum 
{
    UART_PACKET_TYPE_IGNORE     = 0,
    UART_PACKET_TYPE_EXECUTE    = 1,
    UART_PACKET_TYPE_SECTION    = 2,
    UART_PACKET_TYPE_MEMSET     = 3,
    UART_PACKET_TYPE_MEMCPY     = 4,
};

void handle_uart_execute_packet(uart_packet_header* base_header);
void handle_uart_section_packet(uart_packet_header* base_header);
void handle_uart_memset_packet(uart_packet_header* base_header);
void handle_uart_memcpy_packet(uart_packet_header* base_header);

void send_magic_word();

int parse_uart_packets();


int main() 
{
    uart_keypad_emmulation(0);

    printf("Uart application copying ready\n");    
    
    return parse_uart_packets();
}

void send_magic_word()
{
    for (int i = 0 ; i < (sizeof(ready_magic_number) - 1); i++)
        uart_putc(ready_magic_number[i]);
}

int parse_uart_packets()
{
    uart_packet_header header;

    while (1)
    {
        send_magic_word();              // Tell the client that we are ready for the next packet
        header.type = uart_getc();
        int data_size = 0;

        switch (header.type)
        {
        case UART_PACKET_TYPE_IGNORE:
            break;                      
        case UART_PACKET_TYPE_EXECUTE:
            data_size = sizeof(uart_execute_packet_header);
            break;
        case UART_PACKET_TYPE_SECTION:
            data_size = sizeof(uart_section_packet_header);
            break;
        case UART_PACKET_TYPE_MEMSET:
            data_size = sizeof(uart_memset_packet_header);
            break;
        case UART_PACKET_TYPE_MEMCPY:
            data_size = sizeof(uart_memcpy_packet_header);
            break;
        default:
            printf("Uknown packet type: %d, aborting!\n", header.type);
            return -1;
        }

        for (int i = 0; i < data_size; i++)
        {
            header.data[i] = uart_getc();
        }

        switch (header.type)
        {
        case UART_PACKET_TYPE_IGNORE:       // This one does nothing, and that is intentional
            break;                      
        case UART_PACKET_TYPE_EXECUTE:
            handle_uart_execute_packet(&header);
            break;
        case UART_PACKET_TYPE_SECTION:
            handle_uart_section_packet(&header);
            break;
        case UART_PACKET_TYPE_MEMSET:
            handle_uart_memset_packet(&header);
            break;
        case UART_PACKET_TYPE_MEMCPY:
            handle_uart_memcpy_packet(&header);
            break;
        default:
            printf("Uknown packet type: %d, aborting!\n", header.type);
            return -1;
        }
        
        printf("Done!\n");
    }

    return 0;
}

void handle_uart_execute_packet(uart_packet_header* base_header)
{
    uart_execute_packet_header* header = (uart_execute_packet_header*)base_header->data;

    uart_keypad_emmulation(-2); // Set keypad emmulation to its defult state

    asm volatile ("blr %x0"     // Yes this leave the current programs text section lying around
	:                           // But apart from a custom syscall I cant think of a way around it
	: "r" (header->entry)
	: "x0");

    exit(0);                    // In case the loaded app somehow trys to return insted of calling exit its self
}

void handle_uart_section_packet(uart_packet_header* base_header)
{
    uart_section_packet_header* header = (uart_section_packet_header*)base_header->data;
    
    if (vmemmap(header->section_beign, header->section_size, header->flags) < header->section_size)
    {
        printf("Failed to allocate section: 0x%x (0x%x bytes)\n", header->section_beign, header->section_size);
        exit(-2);
    }
}

void handle_uart_memset_packet(uart_packet_header* base_header)
{
    uart_memset_packet_header* header = (uart_memset_packet_header*)base_header->data;

    memset(header->start, header->size, header->value);
    printf("memset 0x%x to 0x%x, for 0x%x bytes\n", header->start, header->value, header->size);
}


void handle_uart_memcpy_packet(uart_packet_header* base_header)
{
    uart_memcpy_packet_header* header = (uart_memcpy_packet_header*)base_header->data;

    uint8_t* ptr = header->start;
    uint8_t* end = ptr + header->size;

    send_magic_word();

    while (ptr != end)
        *ptr++ = uart_getc();

    printf("Copyed %x bytes from uart\n", header->size);
}