#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#include "cell.h"

// Array data management
array_t* create_array_data(context_t* ctx, size_t initial_capacity);
void resize_array_data(context_t* ctx, array_t* array, size_t new_capacity);
void free_array_data(array_t* array);
cell_t new_empty_array(context_t* ctx);

#endif  // ARRAY_H