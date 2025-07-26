#include "core.h"

#include <string.h>

#include "array.h"
#include "context.h"
#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "memory.h"
#include "parser.h"
#include "stack.h"
#include "util.h"

// Global compilation state
bool compilation_mode = false;
cell_array_t* compiling_definition = NULL;
char compiling_word_name[32];

// Stack manipulation words

static void native_dup(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    error(ctx, "DUP: stack underflow");
  }

  cell_t* top = data_peek(ctx, 0);
  data_push_ptr(ctx, top);
}

static void native_drop(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    error(ctx, "DROP: stack underflow");
  }

  cell_t* cell = data_pop(ctx);
  release(cell);
}

static void native_swap(context_t* ctx) {
  require(ctx, 2, "SWAP");

  cell_t* a = data_pop(ctx);
  cell_t b = data_pop_cell(ctx);
  data_push_ptr_no_retain(ctx, a);
  data_push_no_retain(ctx, b);
}

// Arithmetic words

static void native_add(context_t* ctx) {
  require(ctx, 2, "+");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT32 && b->type == CELL_INT32) {
    a->payload.i32 += b->payload.i32;
  } else if (a->type == CELL_FLOAT && b->type == CELL_FLOAT) {
    a->payload.f64 += b->payload.f64;
  } else if (a->type == CELL_INT64 && b->type == CELL_INT64) {
    a->payload.i64 += b->payload.i64;
  } else {
    error(ctx, "+ : type mismatch");
  }
}

static void native_subtract(context_t* ctx) {
  require(ctx, 2, "-");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT32 && b->type == CELL_INT32) {
    // TODO: Check for overflow
    a->payload.i32 -= b->payload.i32;
  } else if (a->type == CELL_FLOAT && b->type == CELL_FLOAT) {
    a->payload.f64 -= b->payload.f64;
  } else if (a->type == CELL_INT64 && b->type == CELL_INT64) {
    // TODO: Check for overflow
    a->payload.i64 -= b->payload.i64;
  } else {
    error(ctx, "- : type mismatch");
  }
}

static void native_multiply(context_t* ctx) {
  require(ctx, 2, "*");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT32 && b->type == CELL_INT32) {
    // TODO: Check for overflow
    a->payload.i32 *= b->payload.i32;
  } else if (a->type == CELL_FLOAT && b->type == CELL_FLOAT) {
    a->payload.f64 *= b->payload.f64;
  } else if (a->type == CELL_INT64 && b->type == CELL_INT64) {
    // TODO: Check for overflow
    a->payload.i64 *= b->payload.i64;
  } else {
    error(ctx, "* : type mismatch");
  }
}

static void native_divide(context_t* ctx) {
  require(ctx, 2, "/");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT32 && b->type == CELL_INT32) {
    if (b->payload.i32 == 0) {
      error(ctx, "/ : division by zero");
    }
    a->payload.i32 /= b->payload.i32;
  } else if (a->type == CELL_FLOAT && b->type == CELL_FLOAT) {
    if (b->payload.f64 == 0.0) {
      error(ctx, "/ : division by zero");
    }
    a->payload.f64 /= b->payload.f64;
  } else if (a->type == CELL_INT64 && b->type == CELL_INT64) {
    if (b->payload.i64 == 0) {
      error(ctx, "/ : division by zero");
    }
    a->payload.i64 /= b->payload.i64;
  } else {
    error(ctx, "/ : type mismatch");
  }
}

static void native_modulo(context_t* ctx) {
  require(ctx, 2, "%");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT32 && b->type == CELL_INT32) {
    if (b->payload.i32 == 0) {
      error(ctx, "% : division by zero");
    }
    a->payload.i32 %= b->payload.i32;
  } else if (a->type == CELL_INT64 && b->type == CELL_INT64) {
    if (b->payload.i64 == 0) {
      error(ctx, "% : division by zero");
    }
    a->payload.i64 %= b->payload.i64;
  } else {
    error(ctx, "% : only works on integer types");
  }
}

// Type conversion words

static void native_to_int32(context_t* ctx) {
  require(ctx, 1, "INT32");

  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT32) {
    // Already INT32, no conversion needed
    return;
  } else if (a->type == CELL_INT64) {
    // TODO: Check for overflow during conversion
    a->payload.i32 = (int32_t)a->payload.i64;
    a->type = CELL_INT32;
  } else if (a->type == CELL_FLOAT) {
    a->payload.i32 = (int32_t)a->payload.f64;
    a->type = CELL_INT32;
  } else {
    error(ctx, "INT32 : cannot convert type to integer");
  }
}

static void native_to_int64(context_t* ctx) {
  require(ctx, 1, "INT64");

  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT64) {
    // Already INT64, no conversion needed
    return;
  } else if (a->type == CELL_INT32) {
    a->payload.i64 = (int64_t)a->payload.i32;
    a->type = CELL_INT64;
  } else if (a->type == CELL_FLOAT) {
    a->payload.i64 = (int64_t)a->payload.f64;
    a->type = CELL_INT64;
  } else {
    error(ctx, "INT64 : cannot convert type to integer");
  }
}

static void native_to_float(context_t* ctx) {
  require(ctx, 1, "FLOAT");

  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_FLOAT) {
    // Already FLOAT, no conversion needed
    return;
  } else if (a->type == CELL_INT32) {
    a->payload.f64 = (double)a->payload.i32;
    a->type = CELL_FLOAT;
  } else if (a->type == CELL_INT64) {
    a->payload.f64 = (double)a->payload.i64;
    a->type = CELL_FLOAT;
  } else {
    error(ctx, "FLOAT : cannot convert type to float");
  }
}

// I/O words

static void native_print(context_t* ctx) {
  require(ctx, 1, "PRINT");

  cell_t* cell = data_pop(ctx);
  print_cell(cell);
  release(cell);
}

// Array words

static void native_nil(context_t* ctx) { data_push(ctx, new_nil()); }

static void native_comma(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(ctx, ", : insufficient stack (need array and element)");
  }

  cell_t element = data_pop_cell(ctx);
  cell_t array_cell = data_pop_cell(ctx);

  if (array_cell.type == CELL_NIL) {
    // Convert NIL to ARRAY with first element
    cell_array_t* data = create_array_data(ctx, 1);

    // Add the element
    data->elements[0] = element;
    data->length = 1;
    retain(&element);  // Array now owns this reference

    // Create new array cell
    cell_t new_array = {0};
    new_array.type = CELL_ARRAY;
    new_array.payload.ptr = data;

    data_push(ctx, new_array);
    release(&array_cell);
  } else if (array_cell.type == CELL_ARRAY) {
    cell_array_t* data = (cell_array_t*)array_cell.payload.ptr;

    // Check if we need to resize
    if (data->length >= data->capacity) {
      data = resize_array_data(ctx, data, data->capacity + 1);
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
    retain(&element);  // Array now owns this reference

    data_push(ctx, array_cell);
  } else {
    error(ctx, ", : can only append to arrays");
    data_push(ctx, array_cell);
    data_push(ctx, element);
    return;
  }

  release(&element);  // We retained it above, so release our reference
}

static void native_length(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    error(ctx, "LENGTH: stack underflow");
    return;
  }

  cell_t array_cell = data_pop_cell(ctx);

  if (array_cell.type == CELL_NIL) {
    data_push(ctx, new_int32(0));
  } else if (array_cell.type == CELL_ARRAY) {
    cell_array_t* data = (cell_array_t*)array_cell.payload.ptr;
    data_push(ctx, new_int32((int32_t)data->length));
  } else {
    error(ctx, "LENGTH: not an array");
    data_push(ctx, array_cell);
    return;
  }

  release(&array_cell);
}

static void native_index(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(ctx, "INDEX: insufficient stack (need array and index)");
    return;
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

  if (array_cell.type == CELL_NIL) {
    error(ctx, "INDEX: cannot index empty array");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  } else if (array_cell.type != CELL_ARRAY) {
    error(ctx, "INDEX: not an array");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  }

  cell_array_t* data = (cell_array_t*)array_cell.payload.ptr;

  if (index < 0 || index >= (int32_t)data->length) {
    error(ctx, "INDEX: index out of bounds");
    data_push(ctx, array_cell);
    data_push(ctx, index_cell);
    return;
  }

  // Create pointer to the element
  cell_t pointer = new_pointer(&data->elements[index]);
  data_push(ctx, pointer);

  release(&array_cell);
  release(&index_cell);
}

static void native_fetch(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    error(ctx, "@ : stack underflow");
    return;
  }

  cell_t pointer_cell = data_pop_cell(ctx);

  if (pointer_cell.type != CELL_POINTER) {
    error(ctx, "@ : not a pointer");
  }

  if (!pointer_cell.payload.pointer) {
    error(ctx, "@ : null pointer");
  }

  // Push a copy of the pointed-to cell
  cell_t value = *pointer_cell.payload.pointer;
  retain(&value);  // We're making a copy, so retain it
  data_push(ctx, value);

  release(&pointer_cell);
}

static void native_store(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(ctx, "! : insufficient stack (need pointer and value)");
    return;
  }

  cell_t value = data_pop_cell(ctx);
  cell_t pointer_cell = data_pop_cell(ctx);

  if (pointer_cell.type != CELL_POINTER) {
    error(ctx, "! : not a pointer");
  }

  if (!pointer_cell.payload.pointer) {
    error(ctx, "! : null pointer");
  }

  // Release the old value and store the new one
  release(pointer_cell.payload.pointer);
  *pointer_cell.payload.pointer = value;
  retain(&value);  // The pointed-to location now owns this reference

  release(&pointer_cell);
  release(&value);  // We retained it above
}

// Comment word
static void native_paren_comment(context_t* ctx) {
  char* comment = parse_until_char(ctx, ')');
  if (!comment) {
    error(ctx, "( : missing closing )");
  }
  // It's a comment, so just discard it
  metal_free(comment);
}

static void native_def(context_t* ctx) {
  if (compilation_mode) {
    error(ctx, "DEF: already in compilation mode");
  }

  // Parse next word as the definition name
  char word_buffer[32];
  token_type_t token_type =
      parse_next_token(&ctx->input_pos, word_buffer, sizeof(word_buffer));
  if (token_type != TOKEN_WORD) {
    error(ctx, "DEF: expected word name");
  }

  // Initialize compilation
  compiling_definition =
      create_array_data(ctx, 8);  // Start with small capacity
  if (!compiling_definition) {
    error(ctx, "DEF: allocation failed");
  }

  strncpy(compiling_word_name, word_buffer, sizeof(compiling_word_name) - 1);
  compiling_word_name[sizeof(compiling_word_name) - 1] = '\0';
  compilation_mode = true;
  debug("Started compiling word '%s'", compiling_word_name);
}

// Better approach for native_end() - don't use NULL function
static void native_end(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "END: not in compilation mode");
  }

  // Add EXIT to the end of the definition
  dictionary_entry_t* exit_word = find_word("EXIT");
  if (!exit_word) {
    error(ctx, "END: EXIT word not found");
  }

  // Add EXIT as the last instruction
  if (compiling_definition->length >= compiling_definition->capacity) {
    compiling_definition = resize_array_data(
        ctx, compiling_definition, compiling_definition->capacity * 2);
    if (!compiling_definition) {
      error(ctx, "END: failed to resize definition");
    }
  }

  compiling_definition->elements[compiling_definition->length] =
      exit_word->definition;
  compiling_definition->length++;
  retain(&exit_word->definition);

  // Create the code cell
  cell_t code_cell = new_code(compiling_definition);

  add_cell(compiling_word_name, code_cell, "User-defined word");

  // Reset compilation state
  compilation_mode = false;
  compiling_definition = NULL;  // Now owned by the dictionary
  compiling_word_name[0] = '\0';

  debug("Finished compiling word '%s'", compiling_word_name);
}

static void native_exit(context_t* ctx) {
  // todo: finish EXIT
  debug("EXIT executed");
}

static void native_true(context_t* ctx) { data_push(ctx, new_boolean(true)); }

static void native_false(context_t* ctx) { data_push(ctx, new_boolean(false)); }

static void native_null(context_t* ctx) { data_push(ctx, new_null()); }

static void native_undefined_check(context_t* ctx) {
  require(ctx, 1, "UNDEFINED?");

  cell_t* item = data_pop(ctx);
  bool is_undefined = (item->type == CELL_UNDEFINED);

  release(item);
  data_push(ctx, new_boolean(is_undefined));
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
  add_native_word("-", native_subtract, "( a b -- c ) Subtract two numbers");
  add_native_word("*", native_multiply, "( a b -- c ) Multiply two numbers");
  add_native_word("/", native_divide, "( a b -- c ) Divide two numbers");
  add_native_word("%", native_modulo, "( a b -- c ) Modulo of two integers");

  // Boolean and null values
  add_native_word("TRUE", native_true, "( -- true ) Push boolean true");
  add_native_word("FALSE", native_false, "( -- false ) Push boolean false");
  add_native_word("NULL", native_null, "( -- null ) Push null value");
  add_native_word("UNDEFINED?", native_undefined_check,
                  "( a -- bool ) Test if value is undefined");

  // Type conversions
  add_native_word("INT32", native_to_int32,
                  "( a -- int32 ) Convert to 32-bit integer");
  add_native_word("INT64", native_to_int64,
                  "( a -- int64 ) Convert to 64-bit integer");
  add_native_word("FLOAT", native_to_float, "( a -- float ) Convert to float");

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
  add_native_word_immediate("DEF", native_def,
                            "( -- ) <name> Start word definition");
  add_native_word_immediate("END", native_end, "( -- ) End word definition");
  add_native_word("EXIT", native_exit, "( -- ) Exit from word definition");
}