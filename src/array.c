#include "array.h"

#include <stddef.h>

#include "cell.h"
#include "debug.h"
#include "memory.h"

// Array data management functions

array_data_t* create_array_data(size_t initial_capacity) {
  if (initial_capacity == 0) initial_capacity = 1;

  size_t alloc_size =
      sizeof(array_data_t) + (initial_capacity * sizeof(cell_t));
  array_data_t* data = metal_alloc(alloc_size);
  if (!data) {
    debug("Failed to allocate array data for capacity %zu", initial_capacity);
    return NULL;
  }

  data->length = 0;
  data->capacity = initial_capacity;
  debug("Created array data with capacity %zu", initial_capacity);
  return data;
}

array_data_t* resize_array_data(array_data_t* data, size_t new_capacity) {
  if (!data) return NULL;
  size_t alloc_size = sizeof(array_data_t) + (new_capacity * sizeof(cell_t));
  array_data_t* new_data = metal_realloc(data, alloc_size);
  if (!new_data) {
    debug("Failed to resize array data from %zu to %zu", data->capacity,
          new_capacity);
    return NULL;
  }

  new_data->capacity = new_capacity;
  debug("Resized array data from capacity %zu to %zu", data->capacity,
        new_capacity);
  return new_data;
}

// Cell creation for arrays

cell_t new_array(size_t initial_capacity) {
  cell_t cell = {0};
  cell.type = CELL_ARRAY;

  array_data_t* data = create_array_data(initial_capacity);
  if (!data) {
    // Return NIL on allocation failure
    cell.type = CELL_NIL;
    return cell;
  }

  cell.payload.ptr = data;
  return cell;
}