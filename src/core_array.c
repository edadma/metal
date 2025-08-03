#include <string.h>

#include "array.h"
#include "context.h"
#include "dictionary.h"
#include "error.h"
#include "object.h"
#include "stack.h"
#include "strings.h"

// Array words

static void native_nil(context_t* ctx) { data_push(ctx, new_empty_array(ctx)); }

static void native_comma(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(ctx, ", : insufficient stack (need array and element)");
  }

  cell_t element = data_pop_cell(ctx);
  cell_t array_cell = data_pop_cell(ctx);

  if (array_cell.type == CELL_ARRAY) {
    array_t* data = array_cell.payload.array;

    // Check if we need to resize
    if (data->length >= data->capacity) {
      resize_array_data(ctx, data, data->capacity + 1);
    }

    // Add the element
    data->elements[data->length] = element;
    data->length++;
    retain(&element);  // Array now owns this reference

    data_push_no_retain(ctx, array_cell);
  } else {
    error(ctx, ", : can only append to arrays");
  }

  release(&element);
}

static void native_length(context_t* ctx) {
  require_params(ctx, 1, "LENGTH");
  cell_t container = data_pop_cell(ctx);  // POP, not peek!

  if (container.type == CELL_ARRAY) {
    array_t* data = container.payload.array;
    int32_t length = data ? (int32_t)data->length : 0;
    data_push(ctx, new_int32(length));

  } else if (container.type == CELL_OBJECT) {
    object_t* obj = container.payload.object;
    int32_t length = obj ? (int32_t)obj->length : 0;
    data_push(ctx, new_int32(length));

  } else if (container.type == CELL_STRING) {
    size_t len = string_length(ctx, &container);
    data_push(ctx, new_int32((int32_t)len));

  } else {
    error(ctx, "LENGTH: argument must be array, object, or string");
  }

  release(&container);  // Don't forget to release!
}

static void native_index(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(ctx, "INDEX: insufficient stack (need array and index)");
  }

  cell_t index_cell = data_pop_cell(ctx);
  cell_t array_cell = data_pop_cell(ctx);

  if (index_cell.type != CELL_INT32) {
    error(ctx, "INDEX: index must be integer");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  }

  int32_t index = index_cell.payload.i32;

  if (array_cell.type != CELL_ARRAY) {
    error(ctx, "INDEX: not an array");
  }

  array_t* data = array_cell.payload.array;

  if (index < 0 || index >= data->length) {
    error(ctx, "INDEX: index out of bounds");
  }

  // Create pointer to the element
  cell_t pointer = new_pointer(&data->elements[index]);
  data_push(ctx, pointer);

  release(&array_cell);
  release(&index_cell);
}

// INDEX@ ( array index -- value ) OR ( object key -- value ) Polymorphic fetch
static void native_index_fetch(context_t* ctx) {
  require_params(ctx, 2, "INDEX@");
  cell_t index_cell = data_pop_cell(ctx);
  cell_t container_cell = data_pop_cell(ctx);

  if (container_cell.type == CELL_ARRAY) {
    // Original array logic
    if (index_cell.type != CELL_INT32) {
      error(ctx, "INDEX@: array index must be integer");
      return;
    }

    int32_t index = index_cell.payload.i32;
    array_t* data = container_cell.payload.array;

    if (index < 0 || index >= data->length) {
      error(ctx, "INDEX@: index out of bounds");
      return;
    }

    // Get the element and push it (data_push will retain it)
    cell_t element = data->elements[index];
    data_push(ctx, element);

  } else if (container_cell.type == CELL_OBJECT) {
    // New object logic
    if (index_cell.type != CELL_STRING) {
      error(ctx, "INDEX@: object index must be string");
      return;
    }

    object_t* obj = container_cell.payload.object;
    if (!obj) {
      // Empty object - return undefined
      data_push(ctx, new_undefined());
    } else {
      cell_t* found = object_get(ctx, obj, &index_cell);
      if (found) {
        data_push(ctx, *found);  // data_push will retain
      } else {
        data_push(ctx, new_undefined());  // Property doesn't exist
      }
    }

  } else {
    error(ctx, "INDEX@: container must be array or object");
    return;
  }

  release(&container_cell);
  release(&index_cell);
}

// INDEX! ( value array index -- ) OR ( value object key -- ) Polymorphic store
static void native_index_store(context_t* ctx) {
  require_params(ctx, 3, "INDEX!");

  cell_t index_cell = data_pop_cell(ctx);
  cell_t container_cell = data_pop_cell(ctx);
  cell_t value_cell = data_pop_cell(ctx);

  if (container_cell.type == CELL_ARRAY) {
    // Original array logic
    if (index_cell.type != CELL_INT32) {
      error(ctx, "INDEX!: array index must be integer");
      return;
    }

    int32_t index = index_cell.payload.i32;
    array_t* data = container_cell.payload.array;

    if (index < 0 || index >= data->length) {
      error(ctx, "INDEX!: index out of bounds");
      return;
    }

    // Release the old value at this position
    release(&data->elements[index]);

    // Store the new value and retain it
    data->elements[index] = value_cell;
    retain(&value_cell);

  } else if (container_cell.type == CELL_OBJECT) {
    // New object logic
    if (index_cell.type != CELL_STRING) {
      error(ctx, "INDEX!: object index must be string");
      return;
    }

    object_t* obj = container_cell.payload.object;
    if (!obj) {
      error(ctx, "INDEX!: cannot set property on empty object");
      return;
    }

    // Use object_set which handles key interning
    object_set(ctx, obj, &index_cell, &value_cell);

  } else {
    error(ctx, "INDEX!: container must be array or object");
    return;
  }

  release(&container_cell);
  release(&index_cell);
  release(&value_cell);  // Release our local copy
}

// Register all core array words
void add_core_array_words(void) {
  // Array operations
  add_native_word("[]", native_nil, "( -- array ) Create empty array");
  add_native_word(",", native_comma, "( array item -- array ) Append item to array");
  add_native_word("LENGTH", native_length, "( array|string -- n ) Get array or string length");
  add_native_word("INDEX", native_index, "( array n -- ptr ) Get pointer to array element");
  add_native_word("INDEX@", native_index_fetch, "( array n -- value ) Fetch array element safely");
  add_native_word("INDEX!", native_index_store, "( value array n -- ) Store array element safely");
}