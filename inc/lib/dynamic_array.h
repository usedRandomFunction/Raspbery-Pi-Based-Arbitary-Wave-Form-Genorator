#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct dynamic_array
{
    size_t number_of_entrys;
    size_t size_of_entry;
    void* ptr;
};

typedef struct dynamic_array dynamic_array;

// Allocates and sets up the dynamic_array struct
// @param size_of_entry The size of the entrys in bytes
// @param n The number of entrys to allocate
// @param header Pointer to the dynamic_array struct
// @return false if failed to allocate buffer
bool initialize_dynamic_array(size_t size_of_entry, size_t n, dynamic_array* header);

// Allocates the array to be n + 1 entrys and inserts as one move
// @param dynamic_array false if failed to allocate buffer
bool insert_dynamic_array(void* entry, size_t index, dynamic_array* header);

// Allocates the array to be new_size entrys
// @param new_size The new size to set the array to
// @param header Pointer to the dynamic_array struct
// @return false if failed to allocate buffer
bool resize_dynamic_array(size_t new_size, dynamic_array* header);

// Allocates the array to be n - 1 entrys and copys the entry in such a way that removes the given entry
// @param index The index to remove
// @param header Pointer to the dynamic_array struct
// @return false if failed to allocate buffer
bool remove_dynamic_array_entry(size_t index, dynamic_array* header);

#endif