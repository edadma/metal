#include <string.h>

#include "array.h"
#include "context.h"
#include "dictionary.h"
#include "error.h"
#include "stack.h"
#include "util.h"

// Boolean words

static void native_true(context_t* ctx) { data_push(ctx, new_boolean(true)); }

static void native_false(context_t* ctx) { data_push(ctx, new_boolean(false)); }

// Logical operators

static void native_and(context_t* ctx) {
  require_params(ctx, 2, "AND");
  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);
  bool result = is_truthy(a) && is_truthy(b);
  release(a);
  release(b);
  data_push(ctx, new_boolean(result));
}

static void native_or(context_t* ctx) {
  require_params(ctx, 2, "OR");
  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);
  bool result = is_truthy(a) || is_truthy(b);
  release(a);
  release(b);
  data_push(ctx, new_boolean(result));
}

static void native_not(context_t* ctx) {
  require_params(ctx, 1, "NOT");
  cell_t* a = data_pop(ctx);
  bool result = !is_truthy(a);

  release(a);

  data_push(ctx, new_boolean(result));
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
  require_params(ctx, 2, "&");

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
  require_params(ctx, 2, "|");

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
  require_params(ctx, 2, "^");

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
  require_params(ctx, 1, "~");

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
  require_params(ctx, 2, "<<");

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
  require_params(ctx, 2, ">>");

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
  require_params(ctx, 2, ">>>");

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

// Register all core logic words
void add_core_logic_words(void) {
  // Boolean values
  add_native_word("TRUE", native_true, "( -- true ) Push boolean true");
  add_native_word("FALSE", native_false, "( -- false ) Push boolean false");

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
}