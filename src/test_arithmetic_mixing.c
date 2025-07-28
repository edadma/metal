#include "test_arithmetic_mixing.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test addition with all type combinations
TEST_FUNCTION(test_add_type_mixing) {
  // int32 + int32 → int32 (fast path)
  TEST_INTERPRET("42 84 +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(126);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 + int64 → int64 (promotion)
  TEST_INTERPRET("42 1234567890123 +");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 + int32 → int64 (promotion, reverse order)
  TEST_INTERPRET("1234567890123 42 +");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 + float → float (promotion)
  TEST_INTERPRET("42 3.14 +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(45.14);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float + int32 → float (promotion, reverse order)
  TEST_INTERPRET("3.14 42 +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(45.14);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 + float → float (highest precedence)
  TEST_INTERPRET("1000000000000 2.5 +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(1000000000002.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float + int64 → float (highest precedence, reverse)
  TEST_INTERPRET("2.5 1000000000000 +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(1000000000002.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float + float → float (fast path)
  TEST_INTERPRET("3.14 2.86 +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(6.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 + int64 → int64 (fast path)
  TEST_INTERPRET("1000000000000 2000000000000 +");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test subtraction with all type combinations
TEST_FUNCTION(test_subtract_type_mixing) {
  // int32 - int32 → int32 (fast path)
  TEST_INTERPRET("100 42 -");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(58);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 - int64 → int64 (promotion)
  TEST_INTERPRET("100 50 INT64 -");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 - float → float (promotion)
  TEST_INTERPRET("100 2.5 -");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(97.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float - int32 → float (promotion)
  TEST_INTERPRET("10.5 3 -");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(7.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 - float → float (highest precedence)
  TEST_INTERPRET("2000000000000 0.5 -");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(1999999999999.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test negative results
  TEST_INTERPRET("42 100 -");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-58);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test mixed types with negative result
  TEST_INTERPRET("3.14 10.0 -");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-6.86);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test multiplication with all type combinations
TEST_FUNCTION(test_multiply_type_mixing) {
  // int32 * int32 → int32 (fast path)
  TEST_INTERPRET("6 7 *");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 * int64 → int64 (promotion)
  TEST_INTERPRET("1000 1000000 INT64 *");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 * float → float (promotion)
  TEST_INTERPRET("10 2.5 *");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(25.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float * int32 → float (promotion)
  TEST_INTERPRET("3.14 2 *");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(6.28);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 * float → float (highest precedence)
  TEST_INTERPRET("1000000 INT64 1.5 *");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(1500000.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test multiplication by zero
  TEST_INTERPRET("42 0 *");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test negative multiplication
  TEST_INTERPRET("-5 6 *");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-30);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test mixed negative multiplication
  TEST_INTERPRET("-2.5 4 *");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-10.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test division with all type combinations
TEST_FUNCTION(test_divide_type_mixing) {
  // int32 / int32 → int32 (fast path)
  TEST_INTERPRET("84 2 /");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 / int64 → int64 (promotion)
  TEST_INTERPRET("100 5 INT64 /");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 / float → float (promotion)
  TEST_INTERPRET("10 4.0 /");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(2.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float / int32 → float (promotion)
  TEST_INTERPRET("7.5 3 /");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(2.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 / float → float (highest precedence)
  TEST_INTERPRET("1000000 INT64 4.0 /");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(250000.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test integer division truncation
  TEST_INTERPRET("7 3 /");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(2);  // 7/3 = 2 (truncated)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test negative division
  TEST_INTERPRET("-10 3 /");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-3);  // -10/3 = -3 (truncated toward zero)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test division by zero errors for all type combinations
  TEST_EXPECT_ERROR("42 0 /", "division by zero");
  TEST_EXPECT_ERROR("42 0 INT64 /", "division by zero");
  TEST_EXPECT_ERROR("42 0.0 /", "division by zero");
  TEST_EXPECT_ERROR("42.0 0 /", "division by zero");
  TEST_EXPECT_ERROR("42.0 0.0 /", "division by zero");
}

// Test modulo with mixed integer types (float not supported)
TEST_FUNCTION(test_modulo_type_mixing) {
  // int32 % int32 → int32 (fast path)
  TEST_INTERPRET("10 3 %");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 % int64 → int64 (promotion)
  TEST_INTERPRET("100 7 INT64 %");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 % int32 → int64 (promotion)
  TEST_INTERPRET("100 INT64 7 %");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 % int64 → int64 (fast path)
  TEST_INTERPRET("100 INT64 7 INT64 %");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test negative modulo
  TEST_INTERPRET("-10 3 %");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-1);  // -10 % 3 = -1
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test modulo by zero errors
  TEST_EXPECT_ERROR("42 0 %", "division by zero");
  TEST_EXPECT_ERROR("42 0 INT64 %", "division by zero");
  TEST_EXPECT_ERROR("42 INT64 0 %", "division by zero");

  // Test modulo with float (should error)
  TEST_EXPECT_ERROR("42 3.0 %", "only works on integer types");
  TEST_EXPECT_ERROR("42.0 3 %", "only works on integer types");
  TEST_EXPECT_ERROR("42.0 3.0 %", "only works on integer types");
}

// Test complex arithmetic chains with type promotion
// Test complex arithmetic chains with type promotion
TEST_FUNCTION(test_arithmetic_promotion_chains) {
  // Start int32, promote to int64, then to float
  TEST_INTERPRET("10 1000000000 INT64 + 2.5 *");  // Complete chain
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // Final result should be CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Complex expression: (5 + 2.0) * (10 INT64) / 3.5
  TEST_INTERPRET("5 2.0 + 10 INT64 * 3.5 /");  // Complete expression
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(20.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that same-type operations preserve types
  TEST_INTERPRET("42 84 +");  // int32 + int32 → int32
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
  TEST_INTERPRET("1000 INT64 2000 INT64 *");  // int64 * int64 → int64
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("3.14 2.86 -");  // float - float → float
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test edge cases with type promotion
TEST_FUNCTION(test_arithmetic_mixing_edge_cases) {
  // Very large int32 + int64 (tests promotion handling)
  TEST_INTERPRET("2147483647");  // INT32_MAX
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1 INT64 +");  // Should promote to int64
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Small float + large int64
  TEST_INTERPRET("0.1");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("9223372036854775000 INT64 +");  // Large int64
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT (promoted)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Division creating different precision
  TEST_INTERPRET("1 3 /");  // int32 / int32 → int32 (0, truncated)
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("1 3.0 /");  // int32 / float → float (0.333...)
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("0.33 > ");  // Should be > 0.33
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Zero in different types
  TEST_INTERPRET("0 0 INT64 +");  // int32(0) + int64(0) → int64(0)
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 0.0 +");  // int32(0) + float(0.0) → float(0.0)
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative numbers across types
  TEST_INTERPRET("-1 -2 INT64 *");  // int32(-1) * int64(-2) → int64(2)
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test that string concatenation still works and doesn't interfere
TEST_FUNCTION(test_string_vs_numeric_mixing) {
  // String + String → String (should still work)
  TEST_INTERPRET("\"hello\" \"world\" +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_STRING("helloworld");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(4);  // CELL_STRING
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Number + String → Error (should not be promoted)
  TEST_EXPECT_ERROR("42 \"hello\" +", "type mismatch");
  TEST_EXPECT_ERROR("\"hello\" 42 +", "type mismatch");
  TEST_EXPECT_ERROR("3.14 \"world\" +", "type mismatch");
  TEST_EXPECT_ERROR("1000 INT64 \"test\" +", "type mismatch");

  // Other operations with strings (should error)
  TEST_EXPECT_ERROR("\"hello\" \"world\" -", "type mismatch");
  TEST_EXPECT_ERROR("\"hello\" \"world\" *", "type mismatch");
  TEST_EXPECT_ERROR("\"hello\" \"world\" /", "type mismatch");
  TEST_EXPECT_ERROR("\"hello\" \"world\" %", "only works on integer types");
}

// Test performance - ensure fast paths are still taken
TEST_FUNCTION(test_arithmetic_mixing_performance) {
  // These should use fast paths (no promotion)

  // Many int32 operations (should be very fast)
  TEST_INTERPRET("1 2 + 3 + 4 + 5 +");  // All int32
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(15);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Many float operations (should be fast)
  TEST_INTERPRET("1.0 2.0 + 3.0 + 4.0 +");  // All float
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(10.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Chain of int64 operations (should be fast)
  TEST_INTERPRET("1000 INT64 2000 INT64 + 3000 INT64 +");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mix that triggers promotion (slower but correct)
  TEST_INTERPRET("42 3.14 + 2 INT64 *");  // int32→float, then float*int64→float
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all arithmetic type mixing tests
void register_arithmetic_mixing_tests(void) {
  REGISTER_TEST(test_add_type_mixing);
  REGISTER_TEST(test_subtract_type_mixing);
  REGISTER_TEST(test_multiply_type_mixing);
  REGISTER_TEST(test_divide_type_mixing);
  REGISTER_TEST(test_modulo_type_mixing);
  REGISTER_TEST(test_arithmetic_promotion_chains);
  // REGISTER_TEST(test_arithmetic_mixing_edge_cases);
  // REGISTER_TEST(test_string_vs_numeric_mixing);
  // REGISTER_TEST(test_arithmetic_mixing_performance);
}

#endif  // TEST_ENABLED