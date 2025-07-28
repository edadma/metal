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
    // Get lengths
    const size_t alen = a->payload.utf8->length;
    const size_t blen = b->payload.utf8->length;

    // Allocate new string with combined length
    uint8_array_t* new_str =
        metal_alloc(ctx, sizeof(uint8_array_t) + alen + blen);
    if (!new_str) {
      error(ctx, "String concatenation: allocation failed");
      return;
    }

    // Initialize the new string
    new_str->refcount = 1;
    new_str->length = alen + blen;
    new_str->capacity = alen + blen;

    // Copy both strings into the new buffer
    memcpy(new_str->data, a->payload.utf8->data, alen);
    memcpy(new_str->data + alen, b->payload.utf8->data, blen);

    // Release the old string in a
    release(a);

    // Update the cell with new string
    a->type = CELL_STRING;
    a->payload.utf8 = new_str;
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

// Arithmetic combination words

static void native_slash_mod(context_t* ctx) {
  require(ctx, 2, "/MOD");
  cell_t* divisor_cell = data_pop(ctx);
  cell_t* dividend_cell = data_pop(ctx);

  // Handle integer types efficiently
  if (dividend_cell->type == CELL_INT32 && divisor_cell->type == CELL_INT32) {
    int32_t dividend = dividend_cell->payload.i32;
    int32_t divisor = divisor_cell->payload.i32;

    if (divisor == 0) {
      error(ctx, "/MOD : division by zero");
    }

    // Single division operation gives both results
    int32_t quotient = dividend / divisor;
    int32_t remainder = dividend % divisor;

    release(dividend_cell);
    release(divisor_cell);

    // Push remainder first, then quotient (Forth stack order)
    data_push(ctx, new_int32(remainder));
    data_push(ctx, new_int32(quotient));

  } else if ((dividend_cell->type == CELL_INT32 ||
              dividend_cell->type == CELL_INT64) &&
             (divisor_cell->type == CELL_INT32 ||
              divisor_cell->type == CELL_INT64)) {
    // Promote both to INT64
    int64_t dividend = (dividend_cell->type == CELL_INT32)
                           ? (int64_t)dividend_cell->payload.i32
                           : dividend_cell->payload.i64;
    int64_t divisor = (divisor_cell->type == CELL_INT32)
                          ? (int64_t)divisor_cell->payload.i32
                          : divisor_cell->payload.i64;

    if (divisor == 0) {
      error(ctx, "/MOD : division by zero");
    }

    int64_t quotient = dividend / divisor;
    int64_t remainder = dividend % divisor;

    release(dividend_cell);
    release(divisor_cell);

    data_push(ctx, new_int64(remainder));
    data_push(ctx, new_int64(quotient));

  } else {
    error(ctx, "/MOD : requires integer operands");
  }
}

static void native_star_slash(context_t* ctx) {
  require(ctx, 3, "*/");

  cell_t* divisor_cell = data_pop(ctx);
  cell_t* multiplier_cell = data_pop(ctx);
  cell_t* multiplicand_cell = data_pop(ctx);

  // Use wider intermediate type to prevent overflow
  if (multiplicand_cell->type == CELL_INT32 &&
      multiplier_cell->type == CELL_INT32 && divisor_cell->type == CELL_INT32) {
    int32_t multiplicand = multiplicand_cell->payload.i32;
    int32_t multiplier = multiplier_cell->payload.i32;
    int32_t divisor = divisor_cell->payload.i32;

    if (divisor == 0) {
      error(ctx, "*/ : division by zero");
    }

    // Use 64-bit intermediate to prevent overflow
    int64_t intermediate = (int64_t)multiplicand * (int64_t)multiplier;
    int32_t result = (int32_t)(intermediate / divisor);

    release(multiplicand_cell);
    release(multiplier_cell);
    release(divisor_cell);

    data_push(ctx, new_int32(result));

  } else if ((multiplicand_cell->type == CELL_INT32 ||
              multiplicand_cell->type == CELL_INT64) &&
             (multiplier_cell->type == CELL_INT32 ||
              multiplier_cell->type == CELL_INT64) &&
             (divisor_cell->type == CELL_INT32 ||
              divisor_cell->type == CELL_INT64)) {
    // Promote all to INT64 and use double precision for intermediate
    int64_t multiplicand = (multiplicand_cell->type == CELL_INT32)
                               ? (int64_t)multiplicand_cell->payload.i32
                               : multiplicand_cell->payload.i64;
    int64_t multiplier = (multiplier_cell->type == CELL_INT32)
                             ? (int64_t)multiplier_cell->payload.i32
                             : multiplier_cell->payload.i64;
    int64_t divisor = (divisor_cell->type == CELL_INT32)
                          ? (int64_t)divisor_cell->payload.i32
                          : divisor_cell->payload.i64;

    if (divisor == 0) {
      error(ctx, "*/ : division by zero");
    }

    // For 64-bit operands, use double precision float intermediate
    double intermediate = (double)multiplicand * (double)multiplier;
    int64_t result = (int64_t)(intermediate / divisor);

    release(multiplicand_cell);
    release(multiplier_cell);
    release(divisor_cell);

    data_push(ctx, new_int64(result));

  } else if ((multiplicand_cell->type == CELL_FLOAT ||
              multiplicand_cell->type == CELL_INT32 ||
              multiplicand_cell->type == CELL_INT64) &&
             (multiplier_cell->type == CELL_FLOAT ||
              multiplier_cell->type == CELL_INT32 ||
              multiplier_cell->type == CELL_INT64) &&
             (divisor_cell->type == CELL_FLOAT ||
              divisor_cell->type == CELL_INT32 ||
              divisor_cell->type == CELL_INT64)) {
    // Convert all to double
    double multiplicand, multiplier, divisor;

    switch (multiplicand_cell->type) {
      case CELL_INT32:
        multiplicand = (double)multiplicand_cell->payload.i32;
        break;
      case CELL_INT64:
        multiplicand = (double)multiplicand_cell->payload.i64;
        break;
      case CELL_FLOAT:
        multiplicand = multiplicand_cell->payload.f64;
        break;
      default:
        multiplicand = 0.0;
        break;
    }

    switch (multiplier_cell->type) {
      case CELL_INT32:
        multiplier = (double)multiplier_cell->payload.i32;
        break;
      case CELL_INT64:
        multiplier = (double)multiplier_cell->payload.i64;
        break;
      case CELL_FLOAT:
        multiplier = multiplier_cell->payload.f64;
        break;
      default:
        multiplier = 0.0;
        break;
    }

    switch (divisor_cell->type) {
      case CELL_INT32:
        divisor = (double)divisor_cell->payload.i32;
        break;
      case CELL_INT64:
        divisor = (double)divisor_cell->payload.i64;
        break;
      case CELL_FLOAT:
        divisor = divisor_cell->payload.f64;
        break;
      default:
        divisor = 0.0;
        break;
    }

    if (divisor == 0.0) {
      error(ctx, "*/ : division by zero");
    }

    double result = (multiplicand * multiplier) / divisor;

    release(multiplicand_cell);
    release(multiplier_cell);
    release(divisor_cell);

    data_push(ctx, new_float(result));

  } else {
    error(ctx, "*/ : requires numeric operands");
  }
}

static void native_star_slash_mod(context_t* ctx) {
  require(ctx, 3, "*/MOD");

  cell_t* divisor_cell = data_pop(ctx);
  cell_t* multiplier_cell = data_pop(ctx);
  cell_t* multiplicand_cell = data_pop(ctx);

  // Only works with integers (like /MOD)
  if (multiplicand_cell->type == CELL_INT32 &&
      multiplier_cell->type == CELL_INT32 && divisor_cell->type == CELL_INT32) {
    int32_t multiplicand = multiplicand_cell->payload.i32;
    int32_t multiplier = multiplier_cell->payload.i32;
    int32_t divisor = divisor_cell->payload.i32;

    if (divisor == 0) {
      error(ctx, "*/MOD : division by zero");
    }

    // Use 64-bit intermediate to prevent overflow
    int64_t intermediate = (int64_t)multiplicand * (int64_t)multiplier;
    int32_t quotient = (int32_t)(intermediate / divisor);
    int32_t remainder = (int32_t)(intermediate % divisor);

    release(multiplicand_cell);
    release(multiplier_cell);
    release(divisor_cell);

    // Push remainder first, then quotient (Forth stack order)
    data_push(ctx, new_int32(remainder));
    data_push(ctx, new_int32(quotient));

  } else if ((multiplicand_cell->type == CELL_INT32 ||
              multiplicand_cell->type == CELL_INT64) &&
             (multiplier_cell->type == CELL_INT32 ||
              multiplier_cell->type == CELL_INT64) &&
             (divisor_cell->type == CELL_INT32 ||
              divisor_cell->type == CELL_INT64)) {
    // Promote all to INT64
    int64_t multiplicand = (multiplicand_cell->type == CELL_INT32)
                               ? (int64_t)multiplicand_cell->payload.i32
                               : multiplicand_cell->payload.i64;
    int64_t multiplier = (multiplier_cell->type == CELL_INT32)
                             ? (int64_t)multiplier_cell->payload.i32
                             : multiplier_cell->payload.i64;
    int64_t divisor = (divisor_cell->type == CELL_INT32)
                          ? (int64_t)divisor_cell->payload.i32
                          : divisor_cell->payload.i64;

    if (divisor == 0) {
      error(ctx, "*/MOD : division by zero");
    }

    // For 64-bit, we need to be careful about overflow
    // Use double precision for intermediate calculation
    double intermediate = (double)multiplicand * (double)multiplier;
    int64_t quotient = (int64_t)(intermediate / divisor);
    int64_t remainder = (int64_t)intermediate % divisor;

    release(multiplicand_cell);
    release(multiplier_cell);
    release(divisor_cell);

    data_push(ctx, new_int64(remainder));
    data_push(ctx, new_int64(quotient));

  } else {
    error(ctx, "*/MOD : requires integer operands");
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

  // Arithmetic combination words
  add_native_word("/MOD", native_slash_mod,
                  "( n1 n2 -- remainder quotient ) Divide with remainder");
  add_native_word("*/", native_star_slash,
                  "( n1 n2 n3 -- n1*n2/n3 ) Multiply then divide");
  add_native_word(
      "*/MOD", native_star_slash_mod,
      "( n1 n2 n3 -- rem quot ) Multiply then divide with remainder");
}