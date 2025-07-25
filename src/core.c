#include "core.h"

#include "array.h"
#include "dictionary.h"
#include "memory.h"
#include "metal.h"
#include "parser.h"
#include "stack.h"
#include "util.h"

// Stack manipulation words

static void native_dup(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    error("DUP: stack underflow");
    return;
  }

  cell_t top = data_peek(ctx, 0);
  data_push(ctx, top);
}

static void native_drop(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    error("DROP: stack underflow");
    return;
  }

  cell_t cell = data_pop(ctx);
  metal_release(&cell);
}

static void native_swap(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error("SWAP: insufficient stack");
    return;
  }

  cell_t a = data_pop(ctx);
  cell_t b = data_pop(ctx);
  data_push(ctx, a);
  data_push(ctx, b);

  // Release the references we got from data_pop
  metal_release(&a);
  metal_release(&b);
}

// Arithmetic words

static void native_add(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error("+ : insufficient stack");
    return;
  }

  cell_t b = data_pop(ctx);
  cell_t a = data_pop(ctx);

  // Simple integer addition for now
  if (a.type == CELL_INT32 && b.type == CELL_INT32) {
    data_push(ctx, new_int32(a.payload.i32 + b.payload.i32));
  } else {
    error("+ : type mismatch");
    data_push(ctx, a);
    data_push(ctx, b);
  }

  metal_release(&a);
  metal_release(&b);
}

// I/O words

static void native_print(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    error("PRINT: stack underflow");
    return;
  }

  cell_t cell = data_pop(ctx);
  print_cell(&cell);
  metal_release(&cell);
}

// Array words

static void native_nil(context_t* ctx) { data_push(ctx, new_nil()); }

static void native_comma(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(", : insufficient stack (need array and element)");
    return;
  }

  cell_t element = data_pop(ctx);
  cell_t array_cell = data_pop(ctx);

  if (array_cell.type == CELL_NIL) {
    // Convert NIL to ARRAY with first element
    array_data_t* data = create_array_data(1);
    if (!data) {
      data_push(ctx, array_cell);
      data_push(ctx, element);
      return;
    }

    // Add the element
    data->elements[0] = element;
    data->length = 1;
    metal_retain(&element);  // Array now owns this reference

    // Create new array cell
    cell_t new_array = {0};
    new_array.type = CELL_ARRAY;
    new_array.payload.ptr = data;

    data_push(ctx, new_array);
    metal_release(&array_cell);

  } else if (array_cell.type == CELL_ARRAY) {
    array_data_t* data = (array_data_t*)array_cell.payload.ptr;

    // Check if we need to resize
    if (data->length >= data->capacity) {
      data = resize_array_data(data, data->capacity + 1);
      if (!data) {
        data_push(ctx, array_cell);
        data_push(ctx, element);
        return;
      }
      // Update the array cell's pointer (realloc might have moved it)
      array_cell.payload.ptr = data;
    }

    // Add the element
    data->elements[data->length] = element;
    data->length++;
    metal_retain(&element);  // Array now owns this reference

    data_push(ctx, array_cell);

  } else {
    error(", : can only append to arrays");
    data_push(ctx, array_cell);
    data_push(ctx, element);
    return;
  }

  metal_release(&element);  // We retained it above, so release our reference
}

static void native_length(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    error("LENGTH: stack underflow");
    return;
  }

  cell_t array_cell = data_pop(ctx);

  if (array_cell.type == CELL_NIL) {
    data_push(ctx, new_int32(0));
  } else if (array_cell.type == CELL_ARRAY) {
    array_data_t* data = (array_data_t*)array_cell.payload.ptr;
    data_push(ctx, new_int32((int32_t)data->length));
  } else {
    error("LENGTH: not an array");
    data_push(ctx, array_cell);
    return;
  }

  metal_release(&array_cell);
}

static void native_index(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error("INDEX: insufficient stack (need array and index)");
    return;
  }

  cell_t index_cell = data_pop(ctx);
  cell_t array_cell = data_pop(ctx);

  if (index_cell.type != CELL_INT32) {
    error("INDEX: index must be integer");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  }

  int32_t index = index_cell.payload.i32;

  if (array_cell.type == CELL_NIL) {
    error("INDEX: cannot index empty array");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  } else if (array_cell.type != CELL_ARRAY) {
    error("INDEX: not an array");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  }

  array_data_t* data = (array_data_t*)array_cell.payload.ptr;

  if (index < 0 || index >= (int32_t)data->length) {
    error("INDEX: index out of bounds");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  }

  // Create pointer to the element
  cell_t pointer = new_pointer(&data->elements[index]);
  data_push(ctx, pointer);

  metal_release(&array_cell);
  metal_release(&index_cell);
}

static void native_fetch(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    error("@ : stack underflow");
    return;
  }

  cell_t pointer_cell = data_pop(ctx);

  if (pointer_cell.type != CELL_POINTER) {
    error("@ : not a pointer");
    data_push(ctx, pointer_cell);
    return;
  }

  if (!pointer_cell.payload.pointer) {
    error("@ : null pointer");
    data_push(ctx, pointer_cell);
    return;
  }

  // Push a copy of the pointed-to cell
  cell_t value = *pointer_cell.payload.pointer;
  metal_retain(&value);  // We're making a copy, so retain it
  data_push(ctx, value);

  metal_release(&pointer_cell);
}

static void native_store(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error("! : insufficient stack (need pointer and value)");
    return;
  }

  cell_t value = data_pop(ctx);
  cell_t pointer_cell = data_pop(ctx);

  if (pointer_cell.type != CELL_POINTER) {
    error("! : not a pointer");
    data_push(ctx, pointer_cell);
    data_push(ctx, value);
    return;
  }

  if (!pointer_cell.payload.pointer) {
    error("! : null pointer");
    data_push(ctx, pointer_cell);
    data_push(ctx, value);
    return;
  }

  // Release the old value and store the new one
  metal_release(pointer_cell.payload.pointer);
  *pointer_cell.payload.pointer = value;
  metal_retain(&value);  // The pointed-to location now owns this reference

  metal_release(&pointer_cell);
  metal_release(&value);  // We retained it above
}

// Comment word
static void native_paren_comment(context_t* ctx) {
  char* comment = parse_until_char(ctx, ')');
  if (!comment) {
    error("( : missing closing )");
    return;
  }
  // It's a comment, so just discard it
  metal_free(comment);
}

// Register all core words
void add_core_words(void) {
  // Stack manipulation
  add_native_word("DUP", native_dup, "( a -- a a ) Duplicate top of stack");
  add_native_word("DROP", native_drop, "( a -- ) Remove top of stack");
  add_native_word("SWAP", native_swap,
                  "( a b -- b a ) Swap top two stack items");
  // Arithmetic
  add_native_word("+", native_add, "( a b -- c ) Add two numbers");

  // I/O
  add_native_word("PRINT", native_print, "( a -- ) Print value to output");

  // Array operations
  add_native_word("[]", native_nil, "( -- array ) Create empty array");
  add_native_word(",", native_comma,
                  "( array item -- array ) Append item to array");
  add_native_word("LENGTH", native_length, "( array -- n ) Get array length");
  add_native_word("INDEX", native_index,
                  "( array n -- ptr ) Get pointer to array element");
  add_native_word("@", native_fetch,
                  "( ptr -- value ) Fetch value from pointer");
  add_native_word("!", native_store, "( ptr value -- ) Store value at pointer");

  add_native_word("(", native_paren_comment,
                  "( comment -- ) Parenthesis comment until )");
}