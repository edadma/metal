#include <stdio.h>

#include "dictionary.h"
#include "error.h"
#include "stack.h"
#include "util.h"

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

// Efficient comparison combination words

static void native_zero_equals(context_t* ctx) {
  require(ctx, 1, "0=");
  cell_t* top = data_peek(ctx, 0);
  bool result;

  switch (top->type) {
    case CELL_INT32:
      result = top->payload.i32 == 0;
      break;
    case CELL_INT64:
      result = (top->payload.i64 == 0);
      break;
    case CELL_FLOAT:
      result = (top->payload.f64 == 0.0);
      break;
    default:
      result = false;
  }

  // Replace top of stack with boolean result
  new_boolean_inplace(result, top);
}

static void native_zero_less(context_t* ctx) {
  require(ctx, 1, "0<");
  cell_t* top = data_peek(ctx, 0);
  bool result;
  switch (top->type) {
    case CELL_INT32:
      result = (top->payload.i32 < 0);
      break;
    case CELL_INT64:
      result = (top->payload.i64 < 0);
      break;
    case CELL_FLOAT:
      result = (top->payload.f64 < 0.0);
      break;
    default:
      error(ctx, "0< : requires numeric type");
      return;
  }

  new_boolean_inplace(result, top);
}

static void native_zero_greater(context_t* ctx) {
  require(ctx, 1, "0>");

  cell_t* top = data_peek(ctx, 0);
  bool result;

  switch (top->type) {
    case CELL_INT32:
      result = (top->payload.i32 > 0);
      break;
    case CELL_INT64:
      result = (top->payload.i64 > 0);
      break;
    case CELL_FLOAT:
      result = (top->payload.f64 > 0.0);
      break;
    default:
      error(ctx, "0> : requires numeric type");
      return;
  }

  new_boolean_inplace(result, top);
}

static void native_zero_greater_equal(context_t* ctx) {
  require(ctx, 1, "0>=");
  cell_t* top = data_peek(ctx, 0);
  bool result;
  switch (top->type) {
    case CELL_INT32:
      result = (top->payload.i32 >= 0);
      break;
    case CELL_INT64:
      result = (top->payload.i64 >= 0);
      break;
    case CELL_FLOAT:
      result = (top->payload.f64 >= 0.0);
      break;
    default:
      error(ctx, "0>= : requires numeric type");
      return;
  }

  new_boolean_inplace(result, top);
}

static void native_zero_less_equal(context_t* ctx) {
  require(ctx, 1, "0<=");
  cell_t* top = data_peek(ctx, 0);
  bool result;
  switch (top->type) {
    case CELL_INT32:
      result = (top->payload.i32 <= 0);
      break;
    case CELL_INT64:
      result = (top->payload.i64 <= 0);
      break;
    case CELL_FLOAT:
      result = (top->payload.f64 <= 0.0);
      break;
    default:
      error(ctx, "0<= : requires numeric type");
      return;
  }

  new_boolean_inplace(result, top);
}

static void native_zero_not_equal(context_t* ctx) {
  require(ctx, 1, "0<>");

  cell_t* top = data_peek(ctx, 0);
  bool result;

  switch (top->type) {
    case CELL_INT32:
      result = (top->payload.i32 != 0);
      break;
    case CELL_INT64:
      result = (top->payload.i64 != 0);
      break;
    case CELL_FLOAT:
      result = (top->payload.f64 != 0.0);
      break;
    case CELL_BOOLEAN:
      result = top->payload
                   .boolean;  // true if non-zero (true), false if zero (false)
      break;
    default:
      error(ctx, "0<> : requires numeric or boolean type");
      return;
  }

  new_boolean_inplace(result, top);
}

// Register all core primitive words
void add_core_comparison_words(void) {
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

  // Efficient comparison combination words
  add_native_word("0=", native_zero_equals, "( n -- flag ) Test if zero");
  add_native_word("0<", native_zero_less, "( n -- flag ) Test if negative");
  add_native_word("0>", native_zero_greater, "( n -- flag ) Test if positive");
  add_native_word("0>=", native_zero_greater_equal,
                  "( n -- flag ) Test if >= zero");
  add_native_word("0<=", native_zero_less_equal,
                  "( n -- flag ) Test if <= zero");
  add_native_word("0<>", native_zero_not_equal,
                  "( n -- flag ) Test if not zero");
}