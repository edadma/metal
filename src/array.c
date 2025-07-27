#include "array.h"

#include <stddef.h>

#include "cell.h"
#include "debug.h"
#include "error.h"
#include "memory.h"

// Array data management functions

cell_array_t* create_array_data(context_t* ctx, size_t initial_capacity) {
  if (initial_capacity == 0) initial_capacity = 1;

  size_t alloc_size =
      sizeof(cell_array_t) + (initial_capacity * sizeof(cell_t));
  cell_array_t* data = metal_alloc(ctx, alloc_size);

  if (data) {
    data->refcount = 0;  // no owner yet
    data->length = 0;
    data->capacity = initial_capacity;
    debug("Created array data with capacity %zu, refcount %d", initial_capacity,
          data->refcount);
    return data;
  }

  debug("Failed to allocate array data for capacity %zu", initial_capacity);
  error(ctx, "Failed to allocate array data for capacity %zu",
        initial_capacity);
  return NULL;  // Never reached due to error()
}

cell_array_t* resize_array_data(context_t* ctx, cell_array_t* data,
                                size_t new_capacity) {
  if (!data) return NULL;
  uint32_t saved_refcount = data->refcount;  // Save refcount across realloc

  size_t alloc_size = sizeof(cell_array_t) + (new_capacity * sizeof(cell_t));
  cell_array_t* new_data = metal_realloc(ctx, data, alloc_size);

  if (!new_data) {
    debug("Failed to resize array data from %zu to %zu", data->capacity,
          new_capacity);
    return NULL;
  }

  new_data->refcount = saved_refcount;  // Restore refcount
  new_data->capacity = new_capacity;
  debug("Resized array data from capacity %zu to %zu, refcount %d",
        data->capacity, new_capacity, new_data->refcount);
  return new_data;
}