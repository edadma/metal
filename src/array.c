#include "array.h"

#include <stddef.h>

#include "cell.h"
#include "debug.h"
#include "error.h"
#include "memory.h"

// Array data management functions

cell_array_t* create_array_data(context_t* ctx, size_t initial_capacity) {
  if (initial_capacity == 0) initial_capacity = 1;

  cell_array_t* array = metal_alloc(ctx, sizeof(cell_array_t));

  if (array) {
    array->refcount = 0;
    array->length = 0;
    array->capacity = initial_capacity;
    array->elements = metal_alloc(ctx, initial_capacity * sizeof(cell_t));

    if (array->elements) {
      debug("Created array data with capacity %zu, refcount %d", initial_capacity, array->refcount);
      return array;
    }
  }

  debug("Failed to allocate array data for capacity %zu", initial_capacity);
  error(ctx, "Failed to allocate array data for capacity %zu", initial_capacity);
  return NULL;  // Never reached due to error()
}

cell_array_t* resize_array_data(context_t* ctx, cell_array_t* data, size_t new_capacity) {
  if (!data) return NULL;
  uint32_t saved_refcount = data->refcount;  // Save refcount across realloc

  size_t alloc_size = sizeof(cell_array_t) + (new_capacity * sizeof(cell_t));
  cell_array_t* new_data = metal_realloc(ctx, data, alloc_size);

  if (!new_data) {
    debug("Failed to resize array data from %zu to %zu", data->capacity, new_capacity);
    return NULL;
  }

  new_data->refcount = saved_refcount;  // Restore refcount
  new_data->capacity = new_capacity;
  debug("Resized array data from capacity %zu to %zu, refcount %d", data->capacity, new_capacity, new_data->refcount);
  return new_data;
}

cell_t new_empty_array(context_t* ctx) {
  cell_t cell = {0};
  cell.type = CELL_ARRAY;
  cell.payload.array = create_array_data(ctx, 0);  // Always allocate
  return cell;
}
