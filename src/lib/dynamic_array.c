#include "lib/dynamic_array.h"

#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/math.h"

bool initialize_dynamic_array(size_t size_of_entry, size_t n, dynamic_array* header)
{
    header->number_of_entrys = n;
    header->size_of_entry = size_of_entry;

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

    free(header->ptr);

    header->number_of_entrys++;
    header->ptr = new_buffer;

    return true;
}

bool resize_dynamic_array(size_t new_size, dynamic_array* header)
{
    size_t size = min(header->number_of_entrys, new_size) * header->size_of_entry;
    void* new_buffer = malloc(size);

    if (new_buffer == NULL)
        return false;

    memcpy(new_buffer, header->ptr, size);
    free(header->ptr);

    header->number_of_entrys = new_size;
    header->ptr = new_buffer;

    return true;
}

bool remove_dynamic_array_entry(size_t index, dynamic_array* header)
{
    void* new_buffer = malloc((header->number_of_entrys - 1) * header->size_of_entry);

    if (new_buffer == NULL)
        return false;

    memcpy(new_buffer, header->ptr, index * header->size_of_entry);                 // Copy before the entry
    memcpy(void_ptr_offset_bytes(new_buffer, index * header->size_of_entry),        // New offset of the next entry
        void_ptr_offset_bytes(header->ptr, (index + 1) * header->size_of_entry),    // Offset of the next entry in src
        (header->number_of_entrys - (index + 1)) * header->size_of_entry);          // Number of bytes left
    free(header->ptr);
    
    header->number_of_entrys--;
    header->ptr = new_buffer;

    return true;
}