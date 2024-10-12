#include "lib/dynamic_array.h"

#include "lib/memory.h"
#include "lib/math.h"

bool initialize_dynamic_array(size_t size_of_entry, size_t n, dynamic_array* header)
{
    header->number_of_entrys = n;
    header->size_of_entry = size_of_entry;
    header->ptr = NULL;

    if (n == 0)
        return true;

    header->ptr = malloc(size_of_entry * n);

    return header->ptr != NULL;
}

bool insert_dynamic_array(void* entry, size_t index, dynamic_array* header)
{
    void* new_buffer = malloc((header->number_of_entrys + 1) * header->size_of_entry);

    if (new_buffer == NULL)
        return false;

    memcpy(new_buffer, header->ptr, index * header->size_of_entry);                     // Copy before the entry
    memcpy(new_buffer + index * header->size_of_entry, entry, header->size_of_entry);   // Entry its self
    memcpy(new_buffer + ((index + 1) * header->size_of_entry),                          // Index of entry after inserted
        header->ptr + index * header->size_of_entry,                                    // Index of entry in old buffer at index
        (header->number_of_entrys - index) * header->size_of_entry);                    // Number of bytes to copy

    if (header->ptr != NULL)
        free(header->ptr);

    header->number_of_entrys++;
    header->ptr = new_buffer;

    return true;
}

bool resize_dynamic_array(size_t new_size, dynamic_array* header)
{
    void* new_buffer = NULL;
    if (new_size != 0)
    {
        size_t size = min(header->number_of_entrys, new_size) * header->size_of_entry;


        new_buffer = malloc(size);

        if (new_buffer == NULL)
            return false;

        memcpy(new_buffer, header->ptr, size);
    }
    
    if (header->ptr != NULL)
        free(header->ptr);

    header->number_of_entrys = new_size;
    header->ptr = new_buffer;

    return true;
}

bool remove_dynamic_array_entry(size_t index, dynamic_array* header)
{   
    void* new_buffer = NULL;
    if (header->number_of_entrys > 1)
    {
        new_buffer = malloc((header->number_of_entrys - 1) * header->size_of_entry);

        if (new_buffer == NULL)
            return false;

        memcpy(new_buffer, header->ptr, index * header->size_of_entry);                 // Copy before the entry
        memcpy(void_ptr_offset_bytes(new_buffer, index * header->size_of_entry),        // New offset of the next entry
            void_ptr_offset_bytes(header->ptr, (index + 1) * header->size_of_entry),    // Offset of the next entry in src
            (header->number_of_entrys - (index + 1)) * header->size_of_entry);          // Number of bytes left
        free(header->ptr);
    }
    
    header->number_of_entrys--;
    header->ptr = new_buffer;

    return true;
}

void delete_dynamic_array(dynamic_array* header)
{
    if (header->ptr != NULL)
        free(header->ptr);
}

size_t dynamic_array_find_closest_binary_shearch(dynamic_array* header, void* entry, bool* success, 
    less_then_function less_then, equal_to_function equal_to)
{
    size_t right = header->number_of_entrys - 1;
    size_t left = 0;
    *success = false;

    if (header->number_of_entrys == 0)
        return 0;

    while (left < right)
    {
        size_t middle = left + right;
        middle /= 2;

        if (less_then((header->ptr + header->size_of_entry * middle), entry))
        {
            left = middle + 1;
        }
        else if (equal_to((header->ptr + header->size_of_entry * middle), entry))
        {
            *success = true;
            return middle;
        }
        else
        {
            right = middle - 1;
        }
    }
    
    *success = equal_to((header->ptr + header->size_of_entry * left), entry);
    
    if (*success)
        return left;

    if (less_then((header->ptr + header->size_of_entry * left), entry))
        return left + 1;

    return left;
}

size_t dynamic_array_binary_shearch(dynamic_array* header, void* entry, 
    less_then_function less_then, equal_to_function equal_to)
{
    bool success = false;

    size_t index = dynamic_array_find_closest_binary_shearch(header,
        entry, &success, less_then, equal_to);
    
    if (success)
        return index;

    return -1;
}
