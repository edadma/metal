#include "core.h"

#include <string.h>

#include "array.h"
#include "context.h"
#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "interpreter.h"
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
  debug("executing DUP");

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
  debug("executing +");
  require(ctx, 2, "+");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_peek(ctx, 0);

  if (a->type == CELL_INT32 && b->type == CELL_INT32) {
    a->payload.i32 += b->payload.i32;
  } else if (a->type == CELL_FLOAT && b->type == CELL_FLOAT) {
    a->payload.f64 += b->payload.f64;
  } else if (a->type == CELL_INT64 && b->type == CELL_INT64) {
    a->payload.i64 += b->payload.i64;
  } else if (a->type == CELL_STRING && b->type == CELL_STRING) {
    // String concatenation
    const char* str_a = a->payload.utf8_ptr;
    const char* str_b = b->payload.utf8_ptr;

    if (!str_a) str_a = "";
    if (!str_b) str_b = "";

    size_t len_a = strlen(str_a);
    size_t len_b = strlen(str_b);
    size_t total_len = len_a + len_b;

    char* new_str = metal_alloc(ctx, total_len + 1);
    if (!new_str) {
      error(ctx, "+ : failed to allocate memory for string concatenation");
      return;
    }

    strcpy(new_str, str_a);
    strcat(new_str, str_b);

    // Release the old string in a
    release(a);

    // Update the cell with new string
    a->type = CELL_STRING;
    a->payload.utf8_ptr = new_str;
  } else {
    error(ctx, "+ : type mismatch");
  }

  release(b);
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
    new_array.payload.array = data;

    data_push(ctx, new_array);
    release(&array_cell);
  } else if (array_cell.type == CELL_ARRAY) {
    cell_array_t* data = array_cell.payload.array;

    // Check if we need to resize
    if (data->length >= data->capacity) {
      data = resize_array_data(ctx, data, data->capacity + 1);
      if (!data) {
        data_push(ctx, array_cell);
        data_push(ctx, element);
        return;
      }
      // Update the array cell's pointer (realloc might have moved it)
      array_cell.payload.array = data;
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
    cell_array_t* data = array_cell.payload.array;
    data_push(ctx, new_int32((int32_t)data->length));
  } else if (array_cell.type == CELL_STRING) {
    const char* str = array_cell.payload.utf8_ptr;
    if (!str) {
      data_push(ctx, new_int32(0));  // null string has length 0
    } else {
      data_push(ctx, new_int32((int32_t)strlen(str)));
    }
  } else {
    error(ctx, "LENGTH: not an array or string");
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

  cell_array_t* data = array_cell.payload.array;

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

  if (!pointer_cell.payload.cell_ptr) {
    error(ctx, "@ : null pointer");
  }

  // Push a copy of the pointed-to cell
  cell_t value = *pointer_cell.payload.cell_ptr;
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

  if (!pointer_cell.payload.cell_ptr) {
    error(ctx, "! : null pointer");
  }

  // Release the old value and store the new one
  release(pointer_cell.payload.cell_ptr);
  *pointer_cell.payload.cell_ptr = value;
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
  debug("executing EXIT");
  if (is_return_empty(ctx)) {
    debug("EXIT: no return: set ip = NULL");
    ctx->ip = NULL;
  } else {
    cell_t* ret = return_pop(ctx);

    if (ret->type != CELL_RETURN)
      error(ctx, "Invalid return value");
    else
      ctx->ip = ret->payload.cell_ptr;
  }
}

static void native_true(context_t* ctx) { data_push(ctx, new_boolean(true)); }

static void native_false(context_t* ctx) { data_push(ctx, new_boolean(false)); }

static void native_null(context_t* ctx) { data_push(ctx, new_null()); }

static void native_undefined_check(context_t* ctx) {
  require(ctx, 1, "UNDEFINED?");

  cell_t* item = data_pop(ctx);
  bool is_undefined = item->type == CELL_UNDEFINED;

  data_push(ctx, new_boolean(is_undefined));
}

// Comparison operators

static void native_equal(context_t* ctx) {
  require(ctx, 2, "=");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  bool result = cells_equal(ctx, a, b);

  release(a);
  release(b);

  data_push(ctx, new_boolean(result));
}

static void native_not_equal(context_t* ctx) {
  require(ctx, 2, "!=");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  bool result = !cells_equal(ctx, a, b);

  release(a);
  release(b);

  data_push(ctx, new_boolean(result));
}

static void native_less_than(context_t* ctx) {
  require(ctx, 2, "<");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  int cmp = compare_cells(ctx, a, b);
  bool result = (cmp < 0);

  release(a);
  release(b);

  data_push(ctx, new_boolean(result));
}

static void native_greater_than(context_t* ctx) {
  require(ctx, 2, ">");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  int cmp = compare_cells(ctx, a, b);
  bool result = (cmp > 0);

  release(a);
  release(b);

  data_push(ctx, new_boolean(result));
}

static void native_less_equal(context_t* ctx) {
  require(ctx, 2, "<=");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  int cmp = compare_cells(ctx, a, b);
  bool result = (cmp <= 0);

  release(a);
  release(b);

  data_push(ctx, new_boolean(result));
}

static void native_greater_equal(context_t* ctx) {
  require(ctx, 2, ">=");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  int cmp = compare_cells(ctx, a, b);
  bool result = (cmp >= 0);

  release(a);
  release(b);

  data_push(ctx, new_boolean(result));
}

// Logical operators

static void native_and(context_t* ctx) {
  require(ctx, 2, "AND");
  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);
  bool result = is_truthy(a) && is_truthy(b);
  release(a);
  release(b);
  data_push(ctx, new_boolean(result));
}

static void native_or(context_t* ctx) {
  require(ctx, 2, "OR");
  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);
  bool result = is_truthy(a) || is_truthy(b);
  release(a);
  release(b);
  data_push(ctx, new_boolean(result));
}

static void native_not(context_t* ctx) {
  require(ctx, 1, "NOT");
  cell_t* a = data_pop(ctx);
  bool result = !is_truthy(a);

  release(a);

  data_push(ctx, new_boolean(result));
}

// Control flow compilation words

// IF ( condition -- ) Compile conditional branch
static void native_if(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "IF: only valid during compilation");
  }
  // Compile BRANCH_IF_FALSE with placeholder offset
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH_IF_FALSE;
  branch_cell.payload.i32 = 0;  // Placeholder for back-patching

  int branch_location = compiling_definition->length;

  compile_cell(ctx, branch_cell);
  // Push location onto return stack for back-patching
  return_push(ctx, new_int32(branch_location));
}

// ELSE ( -- ) Compile unconditional branch and back-patch IF
static void native_else(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "ELSE: only valid during compilation");
  }
  if (is_return_empty(ctx)) {
    error(ctx, "ELSE: no matching IF");
  }
  // Compile unconditional BRANCH with placeholder
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = 0;  // Placeholder for back-patching
  int else_location = compiling_definition->length;
  compile_cell(ctx, branch_cell);
  // Back-patch the IF's branch to jump HERE (to the ELSE clause that follows)
  cell_t* if_cell = return_pop(ctx);
  int if_location = if_cell->payload.i32;
  int offset = compiling_definition->length - (if_location + 1);
  compiling_definition->elements[if_location].payload.i32 = offset;
  // Push ELSE location for THEN to patch later
  return_push(ctx, new_int32(else_location));
}

// THEN ( -- ) Back-patch pending branch to jump here
static void native_then(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "THEN: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "THEN: no matching IF or ELSE");
  }

  // Back-patch the pending branch
  cell_t* branch_cell = return_pop(ctx);
  int branch_location = branch_cell->payload.i32;
  int offset = compiling_definition->length - (branch_location + 1);
  compiling_definition->elements[branch_location].payload.i32 = offset;
  release(branch_cell);
}

// Helper function to check if cell is a valid integer type for bitwise ops
static bool is_bitwise_compatible(cell_t* cell) {
  return (cell->type == CELL_INT32 || cell->type == CELL_INT64);
}

// Helper function to get integer value as int64_t for bitwise operations
static int64_t get_int64_value(cell_t* cell) {
  switch (cell->type) {
    case CELL_INT32:
      return (int64_t)cell->payload.i32;
    case CELL_INT64:
      return cell->payload.i64;
    default:
      return 0;  // Should never happen if is_bitwise_compatible was checked
  }
}

// Helper function to determine result type for bitwise operations
static cell_type_t get_bitwise_result_type(cell_t* a, cell_t* b) {
  // If either operand is INT64, result is INT64
  if (a->type == CELL_INT64 || b->type == CELL_INT64) {
    return CELL_INT64;
  }
  return CELL_INT32;
}

// Helper function to create result cell with appropriate type
static cell_t create_bitwise_result(int64_t value, cell_type_t result_type) {
  if (result_type == CELL_INT64) {
    return new_int64(value);
  } else {
    // For INT32, check for overflow (though bitwise ops rarely overflow)
    if (value >= INT32_MIN && value <= INT32_MAX) {
      return new_int32((int32_t)value);
    } else {
      // If it would overflow INT32, promote to INT64
      return new_int64(value);
    }
  }
}

// Bitwise AND
static void native_bit_and(context_t* ctx) {
  require(ctx, 2, "&");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  if (!is_bitwise_compatible(a) || !is_bitwise_compatible(b)) {
    error(ctx, "& : bitwise operations only work on integers");
    return;
  }

  int64_t a_val = get_int64_value(a);
  int64_t b_val = get_int64_value(b);
  int64_t result = a_val & b_val;

  cell_type_t result_type = get_bitwise_result_type(a, b);

  release(a);
  release(b);

  data_push(ctx, create_bitwise_result(result, result_type));
}

// Bitwise OR
static void native_bit_or(context_t* ctx) {
  require(ctx, 2, "|");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  if (!is_bitwise_compatible(a) || !is_bitwise_compatible(b)) {
    error(ctx, "| : bitwise operations only work on integers");
    return;
  }

  int64_t a_val = get_int64_value(a);
  int64_t b_val = get_int64_value(b);
  int64_t result = a_val | b_val;

  cell_type_t result_type = get_bitwise_result_type(a, b);

  release(a);
  release(b);

  data_push(ctx, create_bitwise_result(result, result_type));
}

// Bitwise XOR
static void native_bit_xor(context_t* ctx) {
  require(ctx, 2, "^");

  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  if (!is_bitwise_compatible(a) || !is_bitwise_compatible(b)) {
    error(ctx, "^ : bitwise operations only work on integers");
    return;
  }

  int64_t a_val = get_int64_value(a);
  int64_t b_val = get_int64_value(b);
  int64_t result = a_val ^ b_val;

  cell_type_t result_type = get_bitwise_result_type(a, b);

  release(a);
  release(b);

  data_push(ctx, create_bitwise_result(result, result_type));
}

// Bitwise NOT
static void native_bit_not(context_t* ctx) {
  require(ctx, 1, "~");

  cell_t* a = data_pop(ctx);

  if (!is_bitwise_compatible(a)) {
    error(ctx, "~ : bitwise operations only work on integers");
    return;
  }

  if (a->type == CELL_INT32) {
    int32_t result = ~a->payload.i32;
    release(a);
    data_push(ctx, new_int32(result));
  } else {  // CELL_INT64
    int64_t result = ~a->payload.i64;
    release(a);
    data_push(ctx, new_int64(result));
  }
}

// Left shift
static void native_left_shift(context_t* ctx) {
  require(ctx, 2, "<<");

  cell_t* shift_cell = data_pop(ctx);
  cell_t* value_cell = data_pop(ctx);

  if (!is_bitwise_compatible(value_cell)) {
    error(ctx, "<< : can only shift integers");
    return;
  }

  if (shift_cell->type != CELL_INT32) {
    error(ctx, "<< : shift amount must be a 32-bit integer");
    return;
  }

  int32_t shift_amount = shift_cell->payload.i32;

  // Bounds check shift amount
  if (shift_amount < 0) {
    error(ctx, "<< : shift amount cannot be negative");
    return;
  }

  if (value_cell->type == CELL_INT32) {
    if (shift_amount >= 32) {
      error(ctx, "<< : shift amount too large for 32-bit value");
      return;
    }
    int32_t result = value_cell->payload.i32 << shift_amount;
    release(value_cell);
    release(shift_cell);
    data_push(ctx, new_int32(result));
  } else {  // CELL_INT64
    if (shift_amount >= 64) {
      error(ctx, "<< : shift amount too large for 64-bit value");
      return;
    }
    int64_t result = value_cell->payload.i64 << shift_amount;
    release(value_cell);
    release(shift_cell);
    data_push(ctx, new_int64(result));
  }
}

// Arithmetic right shift (sign-extend)
static void native_right_shift(context_t* ctx) {
  require(ctx, 2, ">>");

  cell_t* shift_cell = data_pop(ctx);
  cell_t* value_cell = data_pop(ctx);

  if (!is_bitwise_compatible(value_cell)) {
    error(ctx, ">> : can only shift integers");
    return;
  }

  if (shift_cell->type != CELL_INT32) {
    error(ctx, ">> : shift amount must be a 32-bit integer");
    return;
  }

  int32_t shift_amount = shift_cell->payload.i32;

  // Bounds check shift amount
  if (shift_amount < 0) {
    error(ctx, ">> : shift amount cannot be negative");
    return;
  }

  if (value_cell->type == CELL_INT32) {
    if (shift_amount >= 32) {
      error(ctx, ">> : shift amount too large for 32-bit value");
      return;
    }
    int32_t result = value_cell->payload.i32 >> shift_amount;
    release(value_cell);
    release(shift_cell);
    data_push(ctx, new_int32(result));
  } else {  // CELL_INT64
    if (shift_amount >= 64) {
      error(ctx, ">> : shift amount too large for 64-bit value");
      return;
    }
    int64_t result = value_cell->payload.i64 >> shift_amount;
    release(value_cell);
    release(shift_cell);
    data_push(ctx, new_int64(result));
  }
}

// Logical right shift (zero-fill)
static void native_logical_right_shift(context_t* ctx) {
  require(ctx, 2, ">>>");

  cell_t* shift_cell = data_pop(ctx);
  cell_t* value_cell = data_pop(ctx);

  if (!is_bitwise_compatible(value_cell)) {
    error(ctx, ">>> : can only shift integers");
    return;
  }

  if (shift_cell->type != CELL_INT32) {
    error(ctx, ">>> : shift amount must be a 32-bit integer");
    return;
  }

  int32_t shift_amount = shift_cell->payload.i32;

  // Bounds check shift amount
  if (shift_amount < 0) {
    error(ctx, ">>> : shift amount cannot be negative");
    return;
  }

  if (value_cell->type == CELL_INT32) {
    if (shift_amount >= 32) {
      error(ctx, ">>> : shift amount too large for 32-bit value");
      return;
    }
    // Cast to unsigned for logical shift
    uint32_t unsigned_val = (uint32_t)value_cell->payload.i32;
    uint32_t unsigned_result = unsigned_val >> shift_amount;
    int32_t result = (int32_t)unsigned_result;

    release(value_cell);
    release(shift_cell);
    data_push(ctx, new_int32(result));
  } else {  // CELL_INT64
    if (shift_amount >= 64) {
      error(ctx, ">>> : shift amount too large for 64-bit value");
      return;
    }
    // Cast to unsigned for logical shift
    uint64_t unsigned_val = (uint64_t)value_cell->payload.i64;
    uint64_t unsigned_result = unsigned_val >> shift_amount;
    int64_t result = (int64_t)unsigned_result;

    release(value_cell);
    release(shift_cell);
    data_push(ctx, new_int64(result));
  }
}

// PICK ( xu ... x1 x0 u -- xu ... x1 x0 xu )
// Copy the u-th item from top of stack (0-indexed)
static void native_pick(context_t* ctx) {
  require(ctx, 1, "PICK");
  cell_t* u_cell = data_pop(ctx);
  if (u_cell->type != CELL_INT32) {
    error(ctx, "PICK: index must be integer");
  }
  int u = u_cell->payload.i32;
  release(u_cell);
  if (u < 0) {
    error(ctx, "PICK: index cannot be negative");
  }
  if (u >= ctx->data_stack_ptr) {
    error(ctx, "PICK: stack underflow");
  }
  // Copy the u-th item (0-indexed from top)
  cell_t* item = data_peek(ctx, u);
  data_push_ptr(ctx, item);
}

// ROLL ( xu xu-1 ... x1 x0 u -- xu-1 ... x1 x0 xu )
// Move the u-th item to top of stack (0-indexed)
static void native_roll(context_t* ctx) {
  require(ctx, 1, "ROLL");
  cell_t* u_cell = data_pop(ctx);
  if (u_cell->type != CELL_INT32) {
    error(ctx, "ROLL: index must be integer");
  }
  int u = u_cell->payload.i32;
  release(u_cell);
  if (u < 0) {
    error(ctx, "ROLL: index cannot be negative");
  }
  if (u == 0) {
    return;  // 0 ROLL is no-op
  }
  if (u >= ctx->data_stack_ptr) {
    error(ctx, "ROLL: stack underflow");
  }
  // Move the u-th item to top
  int source_index = ctx->data_stack_ptr - 1 - u;
  cell_t item = ctx->data_stack[source_index];

  // Shift items down to fill the gap
  for (int i = source_index; i < ctx->data_stack_ptr - 1; i++) {
    ctx->data_stack[i] = ctx->data_stack[i + 1];
  }

  // Put the item on top
  ctx->data_stack[ctx->data_stack_ptr - 1] = item;
}

// BEGIN ( -- ) Mark start of loop
static void native_begin(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "BEGIN: only valid during compilation");
  }
  // Push current location for AGAIN to reference
  return_push(ctx, new_int32(compiling_definition->length));
}

// AGAIN ( -- ) Branch back to matching BEGIN
static void native_again(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "AGAIN: only valid during compilation");
  }
  if (is_return_empty(ctx)) {
    error(ctx, "AGAIN: no matching BEGIN");
  }
  // Get the BEGIN location
  cell_t* begin_cell = return_pop(ctx);
  int begin_location = begin_cell->payload.i32;
  // Calculate offset for unconditional branch back to BEGIN
  int branch_location = compiling_definition->length;
  int offset = begin_location - (branch_location + 1);

  // Compile the branch
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = offset;

  compile_cell(ctx, branch_cell);
  release(begin_cell);
}

// UNTIL ( flag -- ) Branch back to BEGIN if flag is false
static void native_until(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "UNTIL: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "UNTIL: no matching BEGIN");
  }

  // Get the BEGIN location
  cell_t* begin_cell = return_pop(ctx);
  int begin_location = begin_cell->payload.i32;

  // Calculate offset for conditional branch back to BEGIN
  int until_location = compiling_definition->length;
  int offset = begin_location - (until_location + 1);

  // Compile the conditional branch (continues loop if flag is false)
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH_IF_FALSE;
  branch_cell.payload.i32 = offset;

  compile_cell(ctx, branch_cell);
  release(begin_cell);
}

// WHILE ( flag -- ) Continue loop if flag is true, exit if false
static void native_while(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "WHILE: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "WHILE: no matching BEGIN");
  }

  // Compile conditional branch with placeholder (jumps out if false)
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH_IF_FALSE;
  branch_cell.payload.i32 = 0;  // Placeholder for REPEAT to patch

  int while_location = compiling_definition->length;

  compile_cell(ctx, branch_cell);

  // Push WHILE location for REPEAT to patch later
  return_push(ctx, new_int32(while_location));
}

// REPEAT ( -- ) Jump back to BEGIN and patch WHILE's forward jump
static void native_repeat(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "REPEAT: only valid during compilation");
  }

  if (ctx->return_stack_ptr < 2) {
    error(ctx, "REPEAT: no matching BEGIN/WHILE");
  }

  // Pop WHILE location and patch its forward jump to here
  cell_t* while_cell = return_pop(ctx);
  int while_location = while_cell->payload.i32;
  int forward_offset = compiling_definition->length + 1 - (while_location + 1);

  compiling_definition->elements[while_location].payload.i32 = forward_offset;
  // Pop BEGIN location and compile backward jump
  cell_t* begin_cell = return_pop(ctx);
  int begin_location = begin_cell->payload.i32;
  int backward_offset = begin_location - (compiling_definition->length + 1);

  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = backward_offset;
  compile_cell(ctx, branch_cell);

  release(while_cell);
  release(begin_cell);
}

// (DO) ( limit start -- ) Runtime: setup loop parameters
static void native_do_runtime(context_t* ctx) {
  require(ctx, 2, "(DO)");

  cell_t start = data_pop_cell(ctx);
  cell_t limit = data_pop_cell(ctx);

  if (start.type != CELL_INT32 || limit.type != CELL_INT32) {
    error(ctx, "(DO): loop parameters must be integers");
  }

  // Push limit then index to return stack
  return_push(ctx, limit);
  return_push(ctx, start);

  debug("DO: limit=%d start=%d", limit.payload.i32, start.payload.i32);
}

// (LOOP) ( -- ) Runtime: increment by 1 and test boundary
static void native_loop_runtime(context_t* ctx) {
  if (ctx->return_stack_ptr < 2) {
    error(ctx, "(LOOP): return stack underflow");
  }

  // Get current index and limit
  cell_t* index_cell = &ctx->return_stack[ctx->return_stack_ptr - 1];
  cell_t* limit_cell = &ctx->return_stack[ctx->return_stack_ptr - 2];

  int32_t old_index = index_cell->payload.i32;
  int32_t limit = limit_cell->payload.i32;
  int32_t new_index = old_index + 1;

  debug("LOOP: old_index=%d new_index=%d limit=%d", old_index, new_index,
        limit);

  if (new_index >= limit) {
    // Exit loop - clean up return stack and skip the branch
    release(&ctx->return_stack[ctx->return_stack_ptr - 1]);
    release(&ctx->return_stack[ctx->return_stack_ptr - 2]);
    ctx->return_stack_ptr -= 2;
    debug("LOOP: exiting, skipping branch");
    // Skip over the branch instruction
    ctx->ip++;
  } else {
    // Continue loop - update index and execute the branch
    index_cell->payload.i32 = new_index;
    debug("LOOP: continuing, executing branch");
    // Let the branch instruction execute normally
  }
}

// (+LOOP) ( n -- ) Runtime: increment by n and test boundary crossing
static void native_plus_loop_runtime(context_t* ctx) {
  require(ctx, 1, "(+LOOP)");

  if (ctx->return_stack_ptr < 2) {
    error(ctx, "(+LOOP): return stack underflow");
  }

  cell_t increment = data_pop_cell(ctx);
  if (increment.type != CELL_INT32) {
    error(ctx, "(+LOOP): increment must be integer");
  }

  // Get current index and limit
  cell_t* index_cell = &ctx->return_stack[ctx->return_stack_ptr - 1];
  cell_t* limit_cell = &ctx->return_stack[ctx->return_stack_ptr - 2];

  int32_t old_index = index_cell->payload.i32;
  int32_t limit = limit_cell->payload.i32;
  int32_t n = increment.payload.i32;
  int32_t new_index = old_index + n;

  debug("+LOOP: old_index=%d increment=%d new_index=%d limit=%d", old_index, n,
        new_index, limit);

  // ANS Forth boundary crossing logic
  bool terminate;
  if (n >= 0) {
    terminate = (old_index < limit && new_index >= limit);
  } else {
    terminate = (old_index >= limit && new_index < limit);
  }

  if (terminate) {
    // Exit loop - clean up return stack and skip the branch
    release(&ctx->return_stack[ctx->return_stack_ptr - 1]);
    release(&ctx->return_stack[ctx->return_stack_ptr - 2]);
    ctx->return_stack_ptr -= 2;
    debug("+LOOP: exiting, skipping branch");
    // Skip over the branch instruction
    ctx->ip++;
  } else {
    // Continue loop - update index and execute the branch
    index_cell->payload.i32 = new_index;
    debug("+LOOP: continuing, executing branch");
    // Let the branch instruction execute normally
  }

  release(&increment);
}

// DO ( limit start -- ) Compile loop setup
static void native_do(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "DO: only valid during compilation");
  }
  // Compile call to (DO) runtime helper
  dictionary_entry_t* do_runtime = find_word("(DO)");
  if (!do_runtime) {
    error(ctx, "DO: (DO) runtime word not found");
  }
  compile_cell(ctx, do_runtime->definition);
  // Push current location for LOOP/+LOOP to branch back to
  return_push(ctx, new_int32(compiling_definition->length));
}

// LOOP ( -- ) Compile loop increment and branch
static void native_loop(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "LOOP: only valid during compilation");
  }
  if (is_return_empty(ctx)) {
    error(ctx, "LOOP: no matching DO");
  }

  // Compile call to (LOOP) runtime helper
  dictionary_entry_t* loop_runtime = find_word("(LOOP)");
  if (!loop_runtime) {
    error(ctx, "LOOP: (LOOP) runtime word not found");
  }
  compile_cell(ctx, loop_runtime->definition);

  // Compile branch offset back to after DO
  cell_t* do_location_cell = return_pop(ctx);
  int do_location = do_location_cell->payload.i32;
  int offset = do_location - (compiling_definition->length + 1);

  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = offset;
  compile_cell(ctx, branch_cell);

  release(do_location_cell);
}

// +LOOP ( n -- ) Compile loop increment by n and branch
static void native_plus_loop(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "+LOOP: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "+LOOP: no matching DO");
  }

  // Compile call to (+LOOP) runtime helper
  dictionary_entry_t* plus_loop_runtime = find_word("(+LOOP)");
  if (!plus_loop_runtime) {
    error(ctx, "+LOOP: (+LOOP) runtime word not found");
  }
  compile_cell(ctx, plus_loop_runtime->definition);

  // Compile branch offset back to after DO
  cell_t* do_location_cell = return_pop(ctx);
  int do_location = do_location_cell->payload.i32;
  int offset = do_location - (compiling_definition->length + 1);
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = offset;
  compile_cell(ctx, branch_cell);

  release(do_location_cell);
}

// I ( -- index ) Get current loop index
static void native_i(context_t* ctx) {
  if (ctx->return_stack_ptr < 1) {
    error(ctx, "I: no active loop");
  }
  // Index is on top of return stack
  cell_t index = ctx->return_stack[ctx->return_stack_ptr - 1];
  data_push(ctx, index);
}

// J ( -- outer_index ) Get outer loop index
static void native_j(context_t* ctx) {
  if (ctx->return_stack_ptr < 3) {
    error(ctx, "J: no nested loop");
  }

  // Outer index is at depth 2: [..., outer_limit, outer_index, inner_limit,
  // inner_index]
  cell_t outer_index = ctx->return_stack[ctx->return_stack_ptr - 3];
  data_push(ctx, outer_index);
}

// UNLOOP ( -- ) Remove loop parameters from return stack
static void native_unloop(context_t* ctx) {
  if (ctx->return_stack_ptr < 2) {
    error(ctx, "UNLOOP: no active loop");
  }

  // Remove limit and index from return stack
  release(&ctx->return_stack[ctx->return_stack_ptr - 1]);
  release(&ctx->return_stack[ctx->return_stack_ptr - 2]);
  ctx->return_stack_ptr -= 2;
}

// Helper function to add a compiled word definition from source
static void add_definition(const char* name, const char* source,
                           const char* help) {
  // Save current compilation state
  bool saved_compilation_mode = compilation_mode;
  cell_array_t* saved_compiling_definition = compiling_definition;
  char saved_compiling_word_name[32];
  strncpy(saved_compiling_word_name, compiling_word_name,
          sizeof(saved_compiling_word_name));
  // Set up compilation
  compilation_mode = true;
  compiling_definition = create_array_data(&main_context, 8);
  if (!compiling_definition) {
    error(&main_context, "add_definition: allocation failed for %s", name);
  }
  strncpy(compiling_word_name, name, sizeof(compiling_word_name) - 1);
  compiling_word_name[sizeof(compiling_word_name) - 1] = '\0';
  // Compile the source code
  metal_result_t result = interpret(&main_context, source);
  if (result != METAL_OK) {
    // Clean up on error
    if (compiling_definition) {
      for (size_t i = 0; i < compiling_definition->length; i++) {
        release(&compiling_definition->elements[i]);
      }
      metal_free(compiling_definition);
    }
    error(&main_context, "add_definition: failed to compile %s", name);
  }
  // Add EXIT to the end of the definition
  dictionary_entry_t* exit_word = find_word("EXIT");
  if (!exit_word) {
    error(&main_context, "add_definition: EXIT word not found");
  }
  if (compiling_definition->length >= compiling_definition->capacity) {
    compiling_definition =
        resize_array_data(&main_context, compiling_definition,
                          compiling_definition->capacity * 2);
    if (!compiling_definition) {
      error(&main_context, "add_definition: failed to resize definition for %s",
            name);
    }
  }
  compiling_definition->elements[compiling_definition->length] =
      exit_word->definition;
  compiling_definition->length++;
  retain(&exit_word->definition);

  // Create the code cell and add to dictionary
  cell_t code_cell = new_code(compiling_definition);
  add_cell(name, code_cell, help);

  // Restore compilation state
  compilation_mode = saved_compilation_mode;
  compiling_definition = saved_compiling_definition;
  strncpy(compiling_word_name, saved_compiling_word_name,
          sizeof(compiling_word_name));
}

// Register all core words
void add_core_words(void) {
  // Stack manipulation
  add_native_word("DUP", native_dup, "( a -- a a ) Duplicate top of stack");
  add_native_word("DROP", native_drop, "( a -- ) Remove top of stack");
  add_native_word("SWAP", native_swap,
                  "( a b -- b a ) Swap top two stack items");
  add_native_word("PICK", native_pick,
                  "( xu...x1 x0 u -- xu...x1 x0 xu ) Copy u-th item");
  add_native_word("ROLL", native_roll,
                  "( xu...x1 x0 u -- xu-1...x1 x0 xu ) Move u-th item to top");

  // Arithmetic
  add_native_word("+", native_add,
                  "( a b -- c ) Add numbers or concatenate strings");
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

  // Comparison operators
  add_native_word("=", native_equal, "( a b -- bool ) Test equality");
  add_native_word("!=", native_not_equal, "( a b -- bool ) Test inequality");
  add_native_word("<", native_less_than, "( a b -- bool ) Test less than");
  add_native_word(">", native_greater_than,
                  "( a b -- bool ) Test greater than");
  add_native_word("<=", native_less_equal,
                  "( a b -- bool ) Test less than or equal");
  add_native_word(">=", native_greater_equal,
                  "( a b -- bool ) Test greater than or equal");

  // Logical operators
  add_native_word("AND", native_and, "( a b -- bool ) Logical AND");
  add_native_word("OR", native_or, "( a b -- bool ) Logical OR");
  add_native_word("NOT", native_not, "( a -- bool ) Logical NOT");

  // Bitwise operators
  add_native_word("&", native_bit_and, "( a b -- c ) Bitwise AND");
  add_native_word("|", native_bit_or, "( a b -- c ) Bitwise OR");
  add_native_word("^", native_bit_xor, "( a b -- c ) Bitwise XOR");
  add_native_word("~", native_bit_not, "( a -- b ) Bitwise NOT");

  // Shift operators
  add_native_word("<<", native_left_shift,
                  "( value shift -- result ) Left shift");
  add_native_word(">>", native_right_shift,
                  "( value shift -- result ) Arithmetic right shift");
  add_native_word(">>>", native_logical_right_shift,
                  "( value shift -- result ) Logical right shift");

  // I/O
  add_native_word("PRINT", native_print, "( a -- ) Print value to output");

  // Array operations
  add_native_word("[]", native_nil, "( -- array ) Create empty array");
  add_native_word(",", native_comma,
                  "( array item -- array ) Append item to array");
  add_native_word("LENGTH", native_length,
                  "( array|string -- n ) Get array or string length");
  add_native_word("INDEX", native_index,
                  "( array n -- ptr ) Get pointer to array element");
  add_native_word("@", native_fetch,
                  "( ptr -- value ) Fetch value from pointer");
  add_native_word("!", native_store, "( ptr value -- ) Store value at pointer");

  // Control flow (compilation only)
  add_native_word_immediate("IF", native_if, "( bool -- ) Begin conditional");
  add_native_word_immediate("ELSE", native_else, "( -- ) Alternative branch");
  add_native_word_immediate("THEN", native_then, "( -- ) End conditional");

  add_native_word("(", native_paren_comment,
                  "( comment -- ) Parenthesis comment until )");
  add_native_word_immediate("DEF", native_def,
                            "( -- ) <name> Start word definition");
  add_native_word_immediate("END", native_end, "( -- ) End word definition");
  add_native_word("EXIT", native_exit, "( -- ) Exit from word definition");
  add_native_word_immediate("BEGIN", native_begin, "( -- ) Mark start of loop");
  add_native_word_immediate("AGAIN", native_again,
                            "( -- ) Branch back to BEGIN");
  add_native_word_immediate(
      "UNTIL", native_until,
      "( flag -- ) Branch back to BEGIN if flag is false");
  add_native_word_immediate("WHILE", native_while,
                            "( flag -- ) Continue loop if flag is true");
  add_native_word_immediate("REPEAT", native_repeat,
                            "( -- ) Jump back to BEGIN");

  // DO/LOOP constructs
  add_native_word("(DO)", native_do_runtime,
                  "( limit start -- ) Runtime: setup loop");
  add_native_word("(LOOP)", native_loop_runtime,
                  "( -- ) Runtime: increment and test");
  add_native_word("(+LOOP)", native_plus_loop_runtime,
                  "( n -- ) Runtime: increment by n");
  add_native_word_immediate("DO", native_do,
                            "( limit start -- ) Begin counted loop");
  add_native_word_immediate("LOOP", native_loop,
                            "( -- ) End loop, increment by 1");
  add_native_word_immediate("+LOOP", native_plus_loop,
                            "( n -- ) End loop, increment by n");
  add_native_word("I", native_i, "( -- index ) Current loop index");
  add_native_word("J", native_j, "( -- outer_index ) Outer loop index");
  add_native_word("UNLOOP", native_unloop, "( -- ) Remove loop parameters");

  add_definition("OVER", "1 PICK", "( a b -- a b a ) Copy second item to top");
  add_definition("2DUP", "OVER OVER",
                 "( a b -- a b a b ) Duplicate top two items");
  add_definition("MIN", "2DUP > IF SWAP THEN DROP",
                 "( a b -- min ) Return minimum of two numbers");
  add_definition("MAX", "2DUP < IF SWAP THEN DROP",
                 "( a b -- max ) Return maximum of two numbers");
  add_definition("ROT", "2 ROLL", "( a b c -- b c a ) Rotate top three items");
  add_definition("SIGNUM", "DUP 0 < IF DROP -1 ELSE 0 > IF 1 ELSE 0 THEN THEN",
                 "( n -- -1|0|1 ) Return sign of number");
}
