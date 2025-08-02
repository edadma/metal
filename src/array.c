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
}

void resize_array_data(context_t* ctx, cell_array_t* array, size_t new_capacity) {
  array->elements = metal_realloc(ctx, array->elements, new_capacity * sizeof(cell_t));

  if (!array->elements) {
    debug("Failed to allocate array data for capacity %zu", new_capacity);
    error(ctx, "Failed to allocate array data for capacity %zu", new_capacity);
  }

  array->capacity = new_capacity;
}

void free_array_data(cell_array_t* array) {
  // First free the elements array
  metal_free(array->elements);
  // Then free the array structure itself
  metal_free(array);
}

cell_t new_empty_array(context_t* ctx) {
  cell_t cell = {0};
  cell.type = CELL_ARRAY;
  cell.payload.array = create_array_data(ctx, 0);  // Always allocate
  return cell;
}
