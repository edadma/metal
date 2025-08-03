#include "object.h"

#include <stddef.h>
#include <string.h>

#include "cell.h"
#include "debug.h"
#include "error.h"
#include "memory.h"
#include "strings.h"

// Object data management functions

object_t* create_object_data(context_t* ctx, size_t initial_capacity) {
  if (initial_capacity == 0) initial_capacity = 1;

  object_t* object = metal_alloc(ctx, sizeof(object_t));

  if (object) {
    object->refcount = 0;
    object->length = 0;
    object->capacity = initial_capacity;
    object->pairs = metal_alloc(ctx, initial_capacity * sizeof(object_pair_t));

    if (object->pairs) {
      debug("Created object data with capacity %zu, refcount %d", initial_capacity, object->refcount);
      return object;
    }
  }

  debug("Failed to allocate object data for capacity %zu", initial_capacity);
  error(ctx, "Failed to allocate object data for capacity %zu", initial_capacity);
}

void resize_object_data(context_t* ctx, object_t* object, size_t new_capacity) {
  object->pairs = metal_realloc(ctx, object->pairs, new_capacity * sizeof(object_pair_t));

  if (!object->pairs) {
    debug("Failed to allocate object data for capacity %zu", new_capacity);
    error(ctx, "Failed to allocate object data for capacity %zu", new_capacity);
  }

  object->capacity = new_capacity;
}

void free_object_data(object_t* object) {
  // First release all values in the object pairs
  for (size_t i = 0; i < object->length; i++) {
    release(&object->pairs[i].value);
    // Note: keys are interned strings, no need to release
  }

  // Free the pairs array
  metal_free(object->pairs);
  // Then free the object structure itself
  metal_free(object);
}

// Helper function to extract string_t* from either interned or allocated string cells
const string_t* get_string_from_cell(const cell_t* cell) {
  if (cell->type != CELL_STRING) return NULL;
  if (cell->flags & CELL_FLAG_INTERNED) {
    return cell->payload.interned_string;
  }

  return &cell->payload.allocated_string->string;
}

// Find object property by key (handles both interned and non-interned keys)
cell_t* object_get(context_t* ctx, const object_t* obj, const cell_t* key_cell) {
  if (key_cell->type != CELL_STRING) return NULL;
  // Fast path: if key is interned, use pointer comparison
  if (key_cell->flags & CELL_FLAG_INTERNED) {
    const string_t* search_key = key_cell->payload.interned_string;
    for (size_t i = 0; i < obj->length; i++) {
      if (obj->pairs[i].key == search_key) {  // Pointer comparison!
        return &obj->pairs[i].value;
      }
    }
  } else {
    // Slow path: content comparison for non-interned keys
    const string_t* search_str = get_string_from_cell(key_cell);
    for (size_t i = 0; i < obj->length; i++) {
      if (string_equal(obj->pairs[i].key, search_str)) {
        return &obj->pairs[i].value;
      }
    }
  }
  return NULL;  // Key not found
}

// Set object property (always interns the key)
void object_set(context_t* ctx, object_t* obj, const cell_t* key_cell, const cell_t* value_cell) {
  if (key_cell->type != CELL_STRING) {
    error(ctx, "object key must be string");
  }
  const string_t* key_str = get_string_from_cell(key_cell);
  // Always intern the key, even in interpretation mode
  const string_t* interned_key = intern_lookup(ctx, key_str);
  if (!interned_key) {
    interned_key = intern_add(ctx, key_str);
  }

  // Check if key already exists
  for (size_t i = 0; i < obj->length; i++) {
    if (obj->pairs[i].key == interned_key) {  // Pointer comparison with interned key
      // Replace existing value
      release(&obj->pairs[i].value);
      obj->pairs[i].value = *value_cell;
      retain(value_cell);
      return;
    }
  }

  // Key doesn't exist - add new pair
  if (obj->length >= obj->capacity) {
    // Expand capacity (typical doubling strategy)
    size_t new_capacity = obj->capacity ? obj->capacity * 2 : 4;
    resize_object_data(ctx, obj, new_capacity);
  }

  obj->pairs[obj->length].key = interned_key;  // No retain needed - interned strings live forever
  obj->pairs[obj->length].value = *value_cell;
  retain(value_cell);
  obj->length++;
}