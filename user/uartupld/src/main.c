#include "common/program_managment.h"
#include "common/basic_io.h"
#include "common/file_io.h"
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

struct uart_file_create_packet_header
{
    uint64_t size;
}__attribute__((packed));

typedef struct uart_file_create_packet_header uart_file_create_packet_header;


const char ready_magic_number[] = "UARTRDY\n";

enum 
{
    UART_PACKET_TYPE_IGNORE         = 0,
    UART_PACKET_TYPE_FILE_CREATE    = 1,
    UART_PACKET_TYPE_RENAME         = 2,
    UART_PACKET_TYPE_DELETE         = 3,
    UART_PACKET_TYPE_LIST_DIR       = 4,
};

void handle_uart_file_create_packet(uart_packet_header* base_header);
void handle_uart_rename_packet();
void handle_uart_delete_packet();
void handle_uart_list_dir_packet();

int get_path_from_uart(char* path);

void send_magic_word();

int parse_uart_packets();

uint8_t file_buffer[32 * 1024];

int main() 
{
    uart_keypad_emmulation(0);

    printf("Uart file modifaciton ready\n");    
    
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
        case UART_PACKET_TYPE_FILE_CREATE:
            data_size = sizeof(uart_file_create_packet_header);
            break;
        case UART_PACKET_TYPE_RENAME:
        case UART_PACKET_TYPE_DELETE:
        case UART_PACKET_TYPE_LIST_DIR:
            data_size = 0;
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
        case UART_PACKET_TYPE_FILE_CREATE:
            handle_uart_file_create_packet(&header);
            break;
        case UART_PACKET_TYPE_RENAME:
            handle_uart_rename_packet();
            break;
        case UART_PACKET_TYPE_DELETE:
            handle_uart_delete_packet();
            break;
        case UART_PACKET_TYPE_LIST_DIR:
            handle_uart_list_dir_packet();
            break;
        default:
            printf("Uknown packet type: %d, aborting!\n", header.type);
            return -1;
        }
        
        printf("Done!\n");
    }

    return 0;
}

void handle_uart_file_create_packet(uart_packet_header* base_header)
{
    uart_file_create_packet_header* header = (uart_file_create_packet_header*)base_header->data;
    
    char path[256];

    if (get_path_from_uart(path))
    {
        printf("Bad arguments\n");
        return;
    }

    int fd = open(path, FILE_FLAGS_CREATE | FILE_FLAGS_READ_WRITE);

    if (fd == -1)
    {
        printf("Failed to open file\n");
        return;
    }

    uint64_t bytes_remaining = header->size;
    int i;

    while (bytes_remaining)
    {
        send_magic_word();

        for (i = 0; i < (32 * 1024) && bytes_remaining; i++, bytes_remaining--)
            file_buffer[i] = uart_getc();   // Read the file into the buffer 32 KiB at a time

        // Now i = number of bytes ready in buffer therefor

        if (write(fd, file_buffer, i) == -1)
        {
            close(fd);
            {
                printf("Failed to write file\n");
                return;
            }
        }
    }

    ftruncate(fd, header->size);    // Shink the file
    // Handles the case of overwriting a file that is bigger then
    // what we are replacing it with
    
    close(fd);
}

void handle_uart_rename_packet()
{
    char old_path[256];
    char new_path[256];

    if (get_path_from_uart(old_path))
    {
        printf("Bad arguments\n");
        return;
    }

    if (get_path_from_uart(new_path))
    {
        printf("Bad arguments\n");
        return;
    }

    if (rename(old_path, new_path))
    {
        printf("Failed to rename file\n");
        return;
    }
}

void handle_uart_delete_packet()
{
    char path[256];
    
    if (get_path_from_uart(path))
    {
        printf("Bad arguments\n");
        return;
    }

    if (remove(path) == -1)
    {
        printf("Failed to remove file\n");
        return;
    }
}

void handle_uart_list_dir_packet()
{
    char path[256];

    if (get_path_from_uart(path))
    {
        printf("Bad arguments\n");
        return;
    }

    int dir = diropen(path);

    if (dir == -1)
    {
        printf("Failed to open dir\n");
        return;
    }

    dirrectory_entry dir_entry;
    
    printf("%s:\n", path);

    while (dirread(dir, &dir_entry) > 0)
    {
        printf("    %s%c%s: %d bytes %s\n", dir_entry.name, 
                                            (dir_entry.extention[0] == ' ' || 
                                            dir_entry.extention[0] == '\0') ? ' ' : '.',
                                            dir_entry.extention, dir_entry.size, 
                                            dir_entry.size == 0 ? "[dir]" : "[file]");
    }

    dirclose(dir);
}

int get_path_from_uart(char* path)
{
    memclr(path, 256);

    send_magic_word();

    int i = 0;

    for ( ; i < 256; i++)
    {
        path[i] = uart_getc();

        if (path[i] == '\0')
            break;
    }

    if (path[i] != '\0') // Error out if path is not null terimnated
        return -1;

    return 0;
}
