#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#include "cell.h"

// Array data management
cell_array_t* create_array_data(context_t* ctx, size_t initial_capacity);
cell_array_t* resize_array_data(context_t* ctx, cell_array_t* data,
                                size_t new_capacity);

// Cell creation for arrays
cell_t new_array(size_t initial_capacity);

#endif  // ARRAY_H