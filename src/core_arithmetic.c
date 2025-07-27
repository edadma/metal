#include "core_arithmetic.h"

#include <string.h>

#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "memory.h"
#include "stack.h"

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

// Efficient arithmetic combination words

static void native_one_plus(context_t* ctx) {
  require(ctx, 1, "1+");
  cell_t* top = data_peek(ctx, 0);
  switch (top->type) {
    case CELL_INT32:
      // Fast in-place increment - no overflow check for speed
      top->payload.i32++;
      break;
    case CELL_INT64:
      top->payload.i64++;
      break;
    case CELL_FLOAT:
      top->payload.f64 += 1.0;
      break;
    default:
      error(ctx, "1+ : requires numeric type");
  }
}

static void native_one_minus(context_t* ctx) {
  require(ctx, 1, "1-");
  cell_t* top = data_peek(ctx, 0);
  switch (top->type) {
    case CELL_INT32:
      top->payload.i32--;
      break;
    case CELL_INT64:
      top->payload.i64--;
      break;
    case CELL_FLOAT:
      top->payload.f64 -= 1.0;
      break;
    default:
      error(ctx, "1- : requires numeric type");
  }
}

static void native_two_star(context_t* ctx) {
  require(ctx, 1, "2*");
  cell_t* top = data_peek(ctx, 0);
  switch (top->type) {
    case CELL_INT32:
      // Use bit shift for maximum efficiency
      top->payload.i32 <<= 1;
      break;
    case CELL_INT64:
      top->payload.i64 <<= 1;
      break;
    case CELL_FLOAT:
      // Use multiplication for floats
      top->payload.f64 *= 2.0;
      break;
    default:
      error(ctx, "2* : requires numeric type");
  }
}

static void native_two_slash(context_t* ctx) {
  require(ctx, 1, "2/");
  cell_t* top = data_peek(ctx, 0);
  switch (top->type) {
    case CELL_INT32:
      // Use arithmetic right shift (preserves sign)
      top->payload.i32 >>= 1;
      break;
    case CELL_INT64:
      top->payload.i64 >>= 1;
      break;
    case CELL_FLOAT:
      top->payload.f64 /= 2.0;
      break;
    default:
      error(ctx, "2/ : requires numeric type");
  }
}

static void native_negate(context_t* ctx) {
  require(ctx, 1, "NEGATE");
  cell_t* top = data_peek(ctx, 0);
  switch (top->type) {
    case CELL_INT32:
      top->payload.i32 = -top->payload.i32;
      break;
    case CELL_INT64:
      top->payload.i64 = -top->payload.i64;
      break;
    case CELL_FLOAT:
      top->payload.f64 = -top->payload.f64;
      break;
    default:
      error(ctx, "NEGATE : requires numeric type");
  }
}

// Register all core arithmetic words
void add_core_arithmetic_words(void) {
  // Arithmetic
  add_native_word("+", native_add,
                  "( a b -- c ) Add numbers or concatenate strings");
  add_native_word("-", native_subtract, "( a b -- c ) Subtract two numbers");
  add_native_word("*", native_multiply, "( a b -- c ) Multiply two numbers");
  add_native_word("/", native_divide, "( a b -- c ) Divide two numbers");
  add_native_word("%", native_modulo, "( a b -- c ) Modulo of two integers");

  // Type conversions
  add_native_word("INT32", native_to_int32,
                  "( a -- int32 ) Convert to 32-bit integer");
  add_native_word("INT64", native_to_int64,
                  "( a -- int64 ) Convert to 64-bit integer");
  add_native_word("FLOAT", native_to_float, "( a -- float ) Convert to float");

  // Efficient arithmetic combination words
  add_native_word("1+", native_one_plus, "( n -- n+1 ) Add one");
  add_native_word("1-", native_one_minus, "( n -- n-1 ) Subtract one");
  add_native_word("2*", native_two_star, "( n -- n*2 ) Multiply by two");
  add_native_word("2/", native_two_slash, "( n -- n/2 ) Divide by two");
  add_native_word("NEGATE", native_negate, "( n -- -n ) Change sign");
}