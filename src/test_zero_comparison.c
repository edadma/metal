#include "test_zero_comparison.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test 0= (zero equals) with int32 values
TEST_FUNCTION(test_zero_equals_int32) {
  // Test 0= with actual zero
  TEST_INTERPRET("0 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with positive int32
  TEST_INTERPRET("42 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with negative int32
  TEST_INTERPRET("-42 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with 1 and -1
  TEST_INTERPRET("1 0=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("-1 0=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Verify result type is boolean
  TEST_INTERPRET("0 0=");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0= (zero equals) with int64 values
TEST_FUNCTION(test_zero_equals_int64) {
  // Test 0= with int64 zero
  TEST_INTERPRET("0 INT64 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with large positive int64
  TEST_INTERPRET("1234567890123 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with large negative int64
  TEST_INTERPRET("-1234567890123 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with int64 one
  TEST_INTERPRET("1 INT64 0=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0= (zero equals) with float values
TEST_FUNCTION(test_zero_equals_float) {
  // Test 0= with float zero
  TEST_INTERPRET("0.0 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with negative float zero (IEEE 754)
  TEST_INTERPRET("-0.0 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);  // -0.0 should equal 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with positive float
  TEST_INTERPRET("3.14 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with negative float
  TEST_INTERPRET("-2.71 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with very small float
  TEST_INTERPRET("0.000001 0=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-0.000001 0=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0= (zero equals) with boolean values
TEST_FUNCTION(test_zero_equals_boolean) {
  // Test 0= with FALSE (should be false)
  TEST_INTERPRET("FALSE 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0= with TRUE (should be false since TRUE is non-zero-like)
  TEST_INTERPRET("TRUE 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0= mixed types
TEST_FUNCTION(test_zero_mixed_types) {
  // Test 0= with string (false)
  TEST_INTERPRET("\"hello\" 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test 0= with array (false)
  TEST_INTERPRET("[] 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test 0= with null (false)
  TEST_INTERPRET("NULL 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test 0= stack underflow
  TEST_EXPECT_ERROR("0=", "insufficient stack");
}

// Test 0< (zero less than) with int32 values
TEST_FUNCTION(test_zero_less_int32) {
  // Test 0< with negative int32 (should be true)
  TEST_INTERPRET("-42 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with zero (should be false)
  TEST_INTERPRET("0 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with positive int32 (should be false)
  TEST_INTERPRET("42 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with -1 and 1
  TEST_INTERPRET("-1 0<");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("1 0<");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0< (zero less than) with int64 values
TEST_FUNCTION(test_zero_less_int64) {
  // Test 0< with negative int64
  TEST_INTERPRET("-1234567890123 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with zero int64
  TEST_INTERPRET("0 INT64 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with positive int64
  TEST_INTERPRET("1234567890123 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0< (zero less than) with float values
TEST_FUNCTION(test_zero_less_float) {
  // Test 0< with negative float
  TEST_INTERPRET("-3.14 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with zero float
  TEST_INTERPRET("0.0 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with negative zero float
  TEST_INTERPRET("-0.0 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);  // -0.0 is not less than zero
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with positive float
  TEST_INTERPRET("2.71 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0< with very small negative float
  TEST_INTERPRET("-0.000001 0<");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0< error conditions
TEST_FUNCTION(test_zero_less_errors) {
  // Test 0< with non-numeric types
  TEST_EXPECT_ERROR("\"hello\" 0<", "requires numeric");
  TEST_EXPECT_ERROR("TRUE 0<", "requires numeric");
  TEST_EXPECT_ERROR("[] 0<", "requires numeric");

  // Test 0< stack underflow
  TEST_EXPECT_ERROR("0<", "insufficient stack");
}

// Test 0> (zero greater than) with various types
TEST_FUNCTION(test_zero_greater) {
  // Test 0> with positive int32 (should be true)
  TEST_INTERPRET("42 0>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0> with zero (should be false)
  TEST_INTERPRET("0 0>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0> with negative int32 (should be false)
  TEST_INTERPRET("-42 0>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0> with positive int64
  TEST_INTERPRET("1234567890123 0>");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test 0> with negative int64
  TEST_INTERPRET("-1234567890123 0>");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test 0> with positive float
  TEST_INTERPRET("3.14 0>");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test 0> with negative float
  TEST_INTERPRET("-2.71 0>");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test 0> with float zeros
  TEST_INTERPRET("0.0 0>");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-0.0 0>");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0> error conditions
TEST_FUNCTION(test_zero_greater_errors) {
  // Test 0> with non-numeric types
  TEST_EXPECT_ERROR("\"world\" 0>", "requires numeric");
  TEST_EXPECT_ERROR("FALSE 0>", "requires numeric");
  TEST_EXPECT_ERROR("NULL 0>", "requires numeric");

  // Test 0> stack underflow
  TEST_EXPECT_ERROR("0>", "insufficient stack");
}

// Test 0>= (zero greater than or equal) with various types
TEST_FUNCTION(test_zero_greater_equal) {
  // Test 0>= with positive values (should be true)
  TEST_INTERPRET("42 0>=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("1234567890123 0>=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("3.14 0>=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test 0>= with zero (should be true)
  TEST_INTERPRET("0 0>=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 INT64 0>=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0.0 0>=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-0.0 0>=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test 0>= with negative values (should be false)
  TEST_INTERPRET("-42 0>=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-1234567890123 0>=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-2.71 0>=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0>= error conditions
TEST_FUNCTION(test_zero_greater_equal_errors) {
  // Test 0>= with non-numeric types
  TEST_EXPECT_ERROR("\"test\" 0>=", "requires numeric");
  TEST_EXPECT_ERROR("TRUE 0>=", "requires numeric");
  TEST_EXPECT_ERROR("[] 0>=", "requires numeric");

  // Test 0>= stack underflow
  TEST_EXPECT_ERROR("0>=", "insufficient stack");
}

// Test 0<= (zero less than or equal) with various types
TEST_FUNCTION(test_zero_less_equal) {
  // Test 0<= with negative values (should be true)
  TEST_INTERPRET("-42 0<=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-1234567890123 0<=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-3.14 0<=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test 0<= with zero (should be true)
  TEST_INTERPRET("0 0<=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 INT64 0<=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0.0 0<=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-0.0 0<=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test 0<= with positive values (should be false)
  TEST_INTERPRET("42 0<=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("1234567890123 0<=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("2.71 0<=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0<= error conditions
TEST_FUNCTION(test_zero_less_equal_errors) {
  // Test 0<= with non-numeric types
  TEST_EXPECT_ERROR("\"example\" 0<=", "requires numeric");
  TEST_EXPECT_ERROR("FALSE 0<=", "requires numeric");
  TEST_EXPECT_ERROR("NULL 0<=", "requires numeric");

  // Test 0<= stack underflow
  TEST_EXPECT_ERROR("0<=", "insufficient stack");
}

// Test 0!= (zero not equal) with various types
TEST_FUNCTION(test_zero_not_equal) {
  // Test 0!= with zero values (should be false)
  TEST_INTERPRET("0 0!=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 INT64 0!=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0.0 0!=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-0.0 0!=");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test 0!= with non-zero values (should be true)
  TEST_INTERPRET("42 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-42 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("1234567890123 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-1234567890123 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("3.14 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-2.71 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test 0!= with very small values
  TEST_INTERPRET("0.000001 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-0.000001 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0!= (zero not equal) with boolean values
TEST_FUNCTION(test_zero_not_equal_boolean) {
  // Test 0!= with FALSE (should be false since FALSE is zero-like)
  TEST_INTERPRET("FALSE 0!=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 0!= with TRUE (should be true since TRUE is non-zero-like)
  TEST_INTERPRET("TRUE 0!=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 0!= error conditions
TEST_FUNCTION(test_zero_not_equal_errors) {
  // Test 0!= with non-numeric/non-boolean types
  TEST_EXPECT_ERROR("\"string\" 0!=", "requires numeric");
  TEST_EXPECT_ERROR("[] 0!=", "requires numeric");
  TEST_EXPECT_ERROR("NULL 0!=", "requires numeric");

  // Test 0!= stack underflow
  TEST_EXPECT_ERROR("0!=", "insufficient stack");
}

// Test combinations and chains of zero comparisons
// Test combinations and chains of zero comparisons
TEST_FUNCTION(test_zero_comparison_combinations) {
  // Test combination: x 0= NOT is equivalent to x 0!=
  TEST_INTERPRET("42 0= NOT");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("42 0!=");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("=");
  TEST_STACK_TOP_BOOLEAN(true);  // Should be equivalent
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test combination: x 0< NOT AND x 0> NOT is equivalent to x 0=
  TEST_INTERPRET("0");
  TEST_INTERPRET("DUP 0< NOT SWAP DUP 0> NOT AND SWAP 0= =");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Define a helper word for sign detection using conditionals
  TEST_INTERPRET(
      "DEF SIGNUM DUP 0> IF DROP 1 ELSE DUP 0< IF DROP -1 ELSE DROP 0 THEN "
      "THEN END");
  // Test chaining: identify sign of number
  // Positive number
  TEST_INTERPRET("42 SIGNUM");
  TEST_STACK_TOP_INT(1);  // Positive
  TEST_INTERPRET("DROP");
  // Negative number
  TEST_INTERPRET("-42 SIGNUM");
  TEST_STACK_TOP_INT(-1);  // Negative
  TEST_INTERPRET("DROP");

  // Zero
  TEST_INTERPRET("0 SIGNUM");
  TEST_STACK_TOP_INT(0);  // Zero
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test zero comparisons with edge cases
TEST_FUNCTION(test_zero_comparison_edge_cases) {
  // Test with maximum values
  TEST_INTERPRET("2147483647 0>");  // INT32_MAX
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-2147483648 0<");  // INT32_MIN (if supported)
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test with very large int64
  TEST_INTERPRET("9223372036854775807 0>");  // INT64_MAX (if supported)
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test with very small floats
  TEST_INTERPRET("1e-10 0>");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-1e-10 0<");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test precision near zero
  TEST_INTERPRET("0.0000000001 0!=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test that all zero comparison operations return correct boolean type
TEST_FUNCTION(test_zero_comparison_result_types) {
  // Test that all operations return CELL_BOOLEAN
  TEST_INTERPRET("42 0=");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("42 0<");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("42 0>");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("42 0<=");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("42 0>=");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("42 0!=");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all zero comparison tests
void register_zero_comparison_tests(void) {
  REGISTER_TEST(test_zero_equals_int32);
  REGISTER_TEST(test_zero_equals_int64);
  REGISTER_TEST(test_zero_equals_float);
  REGISTER_TEST(test_zero_equals_boolean);
  REGISTER_TEST(test_zero_mixed_types);
  REGISTER_TEST(test_zero_less_int32);
  REGISTER_TEST(test_zero_less_int64);
  REGISTER_TEST(test_zero_less_float);
  REGISTER_TEST(test_zero_less_errors);
  REGISTER_TEST(test_zero_greater);
  REGISTER_TEST(test_zero_greater_errors);
  REGISTER_TEST(test_zero_greater_equal);
  REGISTER_TEST(test_zero_greater_equal_errors);
  REGISTER_TEST(test_zero_less_equal);
  REGISTER_TEST(test_zero_less_equal_errors);
  REGISTER_TEST(test_zero_not_equal);
  REGISTER_TEST(test_zero_not_equal_boolean);
  REGISTER_TEST(test_zero_not_equal_errors);
  REGISTER_TEST(test_zero_comparison_combinations);
  REGISTER_TEST(test_zero_comparison_edge_cases);
  REGISTER_TEST(test_zero_comparison_result_types);
}

#endif  // TEST_ENABLED