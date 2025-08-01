#include "float.h"

#include <math.h>

#include "dictionary.h"
#include "error.h"
#include "stack.h"

// Helper function to convert any numeric cell to double
static double cell_to_double(context_t* ctx, cell_t* cell, const char* word_name) {
  switch (cell->type) {
    case CELL_INT32:
      return (double)cell->payload.i32;
    case CELL_INT64:
      return (double)cell->payload.i64;
    case CELL_FLOAT:
      return cell->payload.f64;
    default:
      error(ctx, "%s: requires numeric type", word_name);
      return 0.0;  // Never reached
  }
}

// Trigonometric functions

static void native_sin(context_t* ctx) {
  require_params(ctx, 1, "SIN");
  cell_t* angle_cell = data_pop(ctx);
  double angle = cell_to_double(ctx, angle_cell, "SIN");
  release(angle_cell);
  double result = sin(angle);
  data_push(ctx, new_float(result));
}

static void native_cos(context_t* ctx) {
  require_params(ctx, 1, "COS");
  cell_t* angle_cell = data_pop(ctx);
  double angle = cell_to_double(ctx, angle_cell, "COS");
  release(angle_cell);
  double result = cos(angle);
  data_push(ctx, new_float(result));
}

static void native_tan(context_t* ctx) {
  require_params(ctx, 1, "TAN");
  cell_t* angle_cell = data_pop(ctx);
  double angle = cell_to_double(ctx, angle_cell, "TAN");
  release(angle_cell);
  double result = tan(angle);
  data_push(ctx, new_float(result));
}

static void native_asin(context_t* ctx) {
  require_params(ctx, 1, "ASIN");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "ASIN");
  release(value_cell);
  if (value < -1.0 || value > 1.0) {
    error(ctx, "ASIN: domain error, argument must be in [-1, 1]");
  }
  double result = asin(value);
  data_push(ctx, new_float(result));
}

static void native_acos(context_t* ctx) {
  require_params(ctx, 1, "ACOS");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "ACOS");
  release(value_cell);
  if (value < -1.0 || value > 1.0) {
    error(ctx, "ACOS: domain error, argument must be in [-1, 1]");
  }
  double result = acos(value);
  data_push(ctx, new_float(result));
}

static void native_atan(context_t* ctx) {
  require_params(ctx, 1, "ATAN");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "ATAN");
  release(value_cell);
  double result = atan(value);
  data_push(ctx, new_float(result));
}

static void native_atan2(context_t* ctx) {
  require_params(ctx, 2, "ATAN2");
  cell_t* x_cell = data_pop(ctx);
  cell_t* y_cell = data_pop(ctx);
  double x = cell_to_double(ctx, x_cell, "ATAN2");
  double y = cell_to_double(ctx, y_cell, "ATAN2");
  release(x_cell);
  release(y_cell);
  double result = atan2(y, x);
  data_push(ctx, new_float(result));
}

// Exponential and logarithmic functions

static void native_exp(context_t* ctx) {
  require_params(ctx, 1, "EXP");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "EXP");
  release(value_cell);
  double result = exp(value);
  data_push(ctx, new_float(result));
}

static void native_ln(context_t* ctx) {
  require_params(ctx, 1, "LN");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "LN");
  release(value_cell);
  if (value <= 0.0) {
    error(ctx, "LN: domain error, argument must be positive");
  }
  double result = log(value);
  data_push(ctx, new_float(result));
}

static void native_log10(context_t* ctx) {
  require_params(ctx, 1, "LOG10");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "LOG10");
  release(value_cell);
  if (value <= 0.0) {
    error(ctx, "LOG10: domain error, argument must be positive");
  }
  double result = log10(value);
  data_push(ctx, new_float(result));
}

static void native_pow(context_t* ctx) {
  require_params(ctx, 2, "POW");
  cell_t* exponent_cell = data_pop(ctx);
  cell_t* base_cell = data_pop(ctx);
  double base = cell_to_double(ctx, base_cell, "POW");
  double exponent = cell_to_double(ctx, exponent_cell, "POW");
  release(base_cell);
  release(exponent_cell);
  // Check for domain errors
  if (base < 0.0 && floor(exponent) != exponent) {
    error(ctx, "POW: domain error, negative base with non-integer exponent");
  }
  if (base == 0.0 && exponent <= 0.0) {
    error(ctx, "POW: domain error, zero base with non-positive exponent");
  }
  double result = pow(base, exponent);
  data_push(ctx, new_float(result));
}

static void native_sqrt(context_t* ctx) {
  require_params(ctx, 1, "SQRT");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "SQRT");
  release(value_cell);
  if (value < 0.0) {
    error(ctx, "SQRT: domain error, argument must be non-negative");
  }
  double result = sqrt(value);
  data_push(ctx, new_float(result));
}

// Rounding functions

static void native_floor(context_t* ctx) {
  require_params(ctx, 1, "FLOOR");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "FLOOR");
  release(value_cell);
  double result = floor(value);
  data_push(ctx, new_float(result));
}

static void native_ceil(context_t* ctx) {
  require_params(ctx, 1, "CEIL");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "CEIL");
  release(value_cell);
  double result = ceil(value);
  data_push(ctx, new_float(result));
}

static void native_round(context_t* ctx) {
  require_params(ctx, 1, "ROUND");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "ROUND");
  release(value_cell);
  double result = round(value);
  data_push(ctx, new_float(result));
}

static void native_trunc(context_t* ctx) {
  require_params(ctx, 1, "TRUNC");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "TRUNC");
  release(value_cell);
  double result = trunc(value);
  data_push(ctx, new_float(result));
}

// Utility functions

static void native_abs(context_t* ctx) {
  require_params(ctx, 1, "ABS");
  cell_t* value_cell = data_pop(ctx);
  double value = cell_to_double(ctx, value_cell, "ABS");
  release(value_cell);
  double result = fabs(value);
  data_push(ctx, new_float(result));
}

static void native_fmod(context_t* ctx) {
  require_params(ctx, 2, "FMOD");
  cell_t* divisor_cell = data_pop(ctx);
  cell_t* dividend_cell = data_pop(ctx);
  double dividend = cell_to_double(ctx, dividend_cell, "FMOD");
  double divisor = cell_to_double(ctx, divisor_cell, "FMOD");
  release(dividend_cell);
  release(divisor_cell);

  if (divisor == 0.0) {
    error(ctx, "FMOD: division by zero");
  }

  double result = fmod(dividend, divisor);
  data_push(ctx, new_float(result));
}

// Mathematical constants

static void native_pi(context_t* ctx) { data_push(ctx, new_float(M_PI)); }

static void native_e(context_t* ctx) { data_push(ctx, new_float(M_E)); }

// Register all floating point words
void add_float_words(void) {
  // Trigonometric functions
  add_native_word("SIN", native_sin, "( angle -- sin ) Sine of angle in radians");
  add_native_word("COS", native_cos, "( angle -- cos ) Cosine of angle in radians");
  add_native_word("TAN", native_tan, "( angle -- tan ) Tangent of angle in radians");
  add_native_word("ASIN", native_asin, "( value -- angle ) Arcsine in radians, domain [-1,1]");
  add_native_word("ACOS", native_acos, "( value -- angle ) Arccosine in radians, domain [-1,1]");
  add_native_word("ATAN", native_atan, "( value -- angle ) Arctangent in radians");
  add_native_word("ATAN2", native_atan2, "( y x -- angle ) Arctangent of y/x in radians");

  // Exponential and logarithmic functions
  add_native_word("EXP", native_exp, "( x -- e^x ) Exponential function");
  add_native_word("LN", native_ln, "( x -- ln(x) ) Natural logarithm, domain (0,∞)");
  add_native_word("LOG10", native_log10, "( x -- log10(x) ) Base-10 logarithm, domain (0,∞)");
  add_native_word("POW", native_pow, "( base exponent -- base^exponent ) Power function");
  add_native_word("SQRT", native_sqrt, "( x -- sqrt(x) ) Square root, domain [0,∞)");

  // Rounding functions
  add_native_word("FLOOR", native_floor, "( x -- floor(x) ) Largest integer ≤ x");
  add_native_word("CEIL", native_ceil, "( x -- ceil(x) ) Smallest integer ≥ x");
  add_native_word("ROUND", native_round, "( x -- round(x) ) Round to nearest integer");
  add_native_word("TRUNC", native_trunc, "( x -- trunc(x) ) Truncate to integer");

  // Utility functions
  add_native_word("ABS", native_abs, "( x -- |x| ) Absolute value");
  add_native_word("FMOD", native_fmod, "( dividend divisor -- remainder ) Floating point remainder");

  // Mathematical constants
  add_native_word("PI", native_pi, "( -- pi ) Mathematical constant π ≈ 3.14159");
  add_native_word("E", native_e, "( -- e ) Mathematical constant e ≈ 2.71828");
}