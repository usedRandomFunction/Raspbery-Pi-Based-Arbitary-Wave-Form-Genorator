#include "lib/config_file.h"

#include "io/file_access.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "io/printf.h"

struct config_file_table_entry
{
    config_file_entry entry;
    size_t hash;
};

typedef struct config_file_table_entry config_file_table_entry;

// Used by binary search function
static bool s_less_then_cfg_file_entry(void* A, void* B);
static bool s_equal_to_cfg_file_entry(void* A, void* B);

// All this function does is load the file into memory
// @param header Pointer to config_file struct to fill out
// @param path The path to the file
// @param end if it is not NULL it is set to the address of the end of the buffer
// @return True if success False if failed
static bool s_read_config_file_from_disk(config_file* header, const char* path, char** end);


bool load_config_file(config_file* header, const char* path)
{
    memclr(header, sizeof(config_file));

    char* end;

    if (s_read_config_file_from_disk(header, path, &end) == false)
        return false;

    initialize_dynamic_array(sizeof(config_file_table_entry), 0, &header->entrys);

    bool comment_active = false;
    bool string_active = false;
    char* name_start = NULL;
    char* name_end = NULL;
    char* value_start = NULL;
    char* value_end = NULL;

    for (char* ptr = header->buffer; ptr < end; ptr++)
    {
        if (comment_active == true)                     // When comment is active
        {
            if (*ptr == '\n')
                comment_active = false;

        }
        else if (*ptr == '#' && string_active == false) // Detect comments
        {
            if (name_start != NULL && value_start == NULL)
            {
                printf("Failed to load config file %s: [%s]\n", path, "Mallformed (Comment before value)");
                printf("[Offset = %d]\n", ptr - header->buffer);
                
                free_loaded_config_file(header);

                return false;
            }

            if (name_start != NULL && value_start != NULL)
            {
                value_end = ptr;
            }

            comment_active = true;
        }
        else if (name_start == NULL)                    // Detect name
        {
            if (*ptr != ' ' && *ptr != '\n')
            {
                name_start = ptr;
            }
        }
        else if (name_end == NULL)                      // Detect end of name
        {
            if (*ptr == ' ' || *ptr == '=')
            {
                name_end = ptr;
            }
        }
        else if (value_start == NULL)                   // Detect start of value
        {
            if (!(*ptr == ' ' || *ptr == '='))
            {
                if (*ptr == '\n')
                {
                    printf("Failed to load config file %s: [%s]\n", path, "Mallformed (Key with no value)");
                    printf("[Offset = %d]\n", ptr - header->buffer);

                    free_loaded_config_file(header);

                    return false;
                }
                value_start = ptr;
                
                if (*ptr == '"')
                {
                    string_active = true;
                    value_start++;
                }
                    

            }
        }
        else if (value_end == NULL)                     // Detect end of a value
        {
            if (!string_active)                         // Quotation marks are not being used
            {
                if (*ptr == '\n' || (ptr + 1) == end)   // New line or end of file
                    value_end = ptr;
            }
            else
            {
                if (*ptr == '"')
                {
                    if (*(ptr - 1) != '\\' ||           // Given its not excsaped
                    (*(ptr - 2) == '\\' && *(ptr - 1) == '\\')) // Or the escape its self is escaped
                    {
                        value_end = ptr;
                        comment_active = true; // To force the program to wait for a new lines
                    }
                }
            }
        }

        if (name_start != NULL && name_end != NULL && 
            value_start != NULL && value_end != NULL) // Ready to insert
        {
            size_t hash = djb2_hash_of_size(name_start, name_end - name_start);

            bool allready_exists = false;

            config_file_table_entry entry;
            entry.entry.back_slash_active = string_active;
            entry.entry.value_begin = value_start;
            entry.entry.value_end = value_end;
            entry.hash = hash;

            size_t index = dynamic_array_find_closest_binary_shearch(&header->entrys,       // Buffer to search
                &entry,                                                                     // Entry to look for
                &allready_exists,                                                           // If it allready exists
                s_less_then_cfg_file_entry, s_equal_to_cfg_file_entry);                     // Less then and equal functions

            if (allready_exists == true)
            {
                printf("Failed to load config file %s: [%s]\n", path, "Naming conflict");
                printf("[Offset = %d]\n", ptr - header->buffer);
                
                free_loaded_config_file(header);

                return false;
            }

            insert_dynamic_array(&entry, index, &header->entrys);        

            string_active = false;
            value_start = NULL;
            name_start = NULL;
            value_end = NULL;
            name_end = NULL;
        }
    }
    

    return true;
}

bool s_read_config_file_from_disk(config_file* header, const char* path, char** end)
{
    int file = open(path, 0);

    if (file == -1)
    {
        printf("Failed to load config file %s: [%s]\n", path, "Failed to open file");
        return false;
    }

    size_t size = get_file_size(file);

    if (size == -1)
    {
        printf("Failed to load config file %s: [%s]\n", path, "Failed to get file size");
        close(file);
        return false;
    }

    header->buffer = malloc(size);
    if (end != NULL) 
        *end = header->buffer + size;

    if ( header->buffer == NULL)
    {
        printf("Failed to load config file %s: [%s]\n", path, "Failed to get allocate buffer");
        close(file);
        return false;
    }

    if (read(file, header->buffer, size) == -1)
    {
        printf("Failed to load config file %s: [%s]\n", path, "Failed to read file");
        close(file);
        return false;
    }
    close(file);

    return true;
}

void free_loaded_config_file(config_file* header)
{
    delete_dynamic_array(&header->entrys);

    if (header->buffer != NULL)
        free(header->buffer);
}

const config_file_entry* get_config_file_entry_from_name(config_file* header, const char* name)
{
    config_file_table_entry _used_in_search;
    _used_in_search.hash = djb2_hash(name);
    
    size_t index = dynamic_array_binary_shearch(&header->entrys,    // Buffer to search
        &_used_in_search,                                           // Entry to look for
        s_less_then_cfg_file_entry, s_equal_to_cfg_file_entry);     // Less then and equal functions

    if (index == -1)
        return NULL;

    config_file_table_entry* entry = (config_file_table_entry*)header->entrys.ptr;
    entry += index;

    return &(entry->entry);
}

char* get_string_from_config_file_entry_allocated(const config_file_entry* entry)
{
    size_t size = entry->value_end - entry->value_begin + 1;

    char* str = malloc(size);

    if (str == NULL)
        return NULL;

    if (!get_string_from_config_file_entry(entry, str, size))
    {
        free(str);

        return NULL;
    }

    return str;
}

bool get_string_from_config_file_entry(const config_file_entry* entry, char* str, size_t dest_size)
{
    char* ptr = entry->value_begin;

    while ((ptr != entry->value_end) && (dest_size--))
    {
        if (entry->back_slash_active)
        {
            if (*ptr == '\\' && *(ptr + 1) != '\\')
            {
                ptr++;
                continue;
            }
        }

        *str++ = *ptr++;
    }
    
    if (dest_size > 0)
        *str = '\0';

    return dest_size != 0;
}

uint64_t get_u64_from_config_file_entry(const config_file_entry* entry, bool* error)
{
    return string_to_u64(entry->value_begin, entry->value_end, error);
}

int64_t get_s64_from_config_file_entry(const config_file_entry* entry, bool* error)
{
    return string_to_s64(entry->value_begin, entry->value_end, error);
}

bool s_less_then_cfg_file_entry(void* A, void* B)
{
    config_file_table_entry* a = (config_file_table_entry*)A;
    config_file_table_entry* b = (config_file_table_entry*)B;

    return a->hash < b->hash;
}

bool s_equal_to_cfg_file_entry(void* A, void* B)
{
    config_file_table_entry* a = (config_file_table_entry*)A;
    config_file_table_entry* b = (config_file_table_entry*)B;

    return a->hash == b->hash;
}
