#include <string.h>

#include "array.h"
#include "context.h"
#include "dictionary.h"
#include "error.h"
#include "stack.h"
#include "strings.h"

// Array words

static void native_nil(context_t* ctx) { data_push(ctx, new_empty_array()); }

static void native_comma(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(ctx, ", : insufficient stack (need array and element)");
  }

  cell_t element = data_pop_cell(ctx);
  cell_t array_cell = data_pop_cell(ctx);

  if (array_cell.type == CELL_ARRAY && !array_cell.payload.array) {
    cell_array_t* data = create_array_data(ctx, 1);

    // Add the element
    data->elements[0] = element;
    data->length = 1;
    retain(&element);  // Array now owns this reference

    // Create new array cell
    cell_t new_array = {0};
    new_array.type = CELL_ARRAY;
    new_array.payload.array = data;

    data_push_ptr(ctx, &new_array);
  } else if (array_cell.type == CELL_ARRAY) {
    cell_array_t* data = array_cell.payload.array;

    // Check if we need to resize
    if (data->length >= data->capacity) {
      data = resize_array_data(ctx, data, data->capacity + 1);
      if (!data) {
        error(ctx, ", : failed to resize array");
      }
      // Update the array cell's pointer (realloc might have moved it)
      array_cell.payload.array = data;
    }

    // Add the element
    data->elements[data->length] = element;
    data->length++;
    retain(&element);  // Array now owns this reference

    data_push_no_retain(ctx, array_cell);
  } else {
    error(ctx, ", : can only append to arrays");
  }

  release(&element);  // We retained it above, so release our reference
}

static void native_length(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    error(ctx, "LENGTH: stack underflow");
    return;
  }

  cell_t cell = data_pop_cell(ctx);

  if (cell.type == CELL_ARRAY && !cell.payload.array) {
    data_push(ctx, new_int32(0));
  } else if (cell.type == CELL_ARRAY) {
    cell_array_t* data = cell.payload.array;
    data_push(ctx, new_int32(data->length));
  } else if (cell.type == CELL_STRING) {
    data_push(ctx, new_int32(string_length(ctx, &cell)));  // null string has length 0
  } else {
    error(ctx, "LENGTH: not an array or string");
  }

  release(&cell);
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

  if (array_cell.type == CELL_ARRAY && !array_cell.payload.array) {
    error(ctx, "INDEX: cannot index empty array");
  } else if (array_cell.type != CELL_ARRAY) {
    error(ctx, "INDEX: not an array");
  }

  cell_array_t* data = array_cell.payload.array;

  if (index < 0 || index >= data->length) {
    error(ctx, "INDEX: index out of bounds");
  }

  // Create pointer to the element
  cell_t pointer = new_pointer(&data->elements[index]);
  data_push(ctx, pointer);

  release(&array_cell);
  release(&index_cell);
}

// Register all core array words
void add_core_array_words(void) {
  // Array operations
  add_native_word("[]", native_nil, "( -- array ) Create empty array");
  add_native_word(",", native_comma, "( array item -- array ) Append item to array");
  add_native_word("LENGTH", native_length, "( array|string -- n ) Get array or string length");
  add_native_word("INDEX", native_index, "( array n -- ptr ) Get pointer to array element");
}