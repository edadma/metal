#ifndef ARRAY_H
#define ARRAY_H

#include "metal.h"

// Array data management
array_data_t* create_array_data(size_t initial_capacity);
array_data_t* resize_array_data(array_data_t* data, size_t new_capacity);

// Cell creation for arrays
cell_t new_array(size_t initial_capacity);

#endif  // ARRAY_H