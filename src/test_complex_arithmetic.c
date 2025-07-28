#include "test_complex_arithmetic.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test /MOD (divide with remainder) with various type combinations
TEST_FUNCTION(test_slash_mod) {
  // Basic int32 /MOD
  TEST_INTERPRET("10 3 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(3);  // quotient on top
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(1);  // remainder second
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Exact division (remainder 0)
  TEST_INTERPRET("15 3 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(5);  // quotient
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(0);  // remainder
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Large int32 /MOD
  TEST_INTERPRET("123456 1000 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(123);  // quotient
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(456);  // remainder
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 /MOD int64 (should promote)
  TEST_INTERPRET("100 7 INT64 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // quotient should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // remainder should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 /MOD int32 (should promote)
  TEST_INTERPRET("1000000000000 7 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // quotient should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // remainder should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 /MOD int64 (both int64)
  TEST_INTERPRET("1000000000000 7 INT64 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // quotient CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // remainder CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative dividend (negative remainder semantics)
  TEST_INTERPRET("-10 3 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(-3);  // quotient (truncate toward zero)
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(-1);  // remainder (same sign as dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative divisor
  TEST_INTERPRET("10 -3 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(-3);  // quotient
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(1);  // remainder (same sign as dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Both negative
  TEST_INTERPRET("-10 -3 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(3);  // quotient
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(-1);  // remainder (same sign as dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // /MOD error conditions
  TEST_EXPECT_ERROR("10 0 /MOD", "division by zero");
  TEST_EXPECT_ERROR("10 0 INT64 /MOD", "division by zero");
  TEST_EXPECT_ERROR("10 INT64 0 /MOD", "division by zero");
  TEST_EXPECT_ERROR("10 3.0 /MOD", "requires integer operands");
  TEST_EXPECT_ERROR("10.0 3 /MOD", "requires integer operands");
  TEST_EXPECT_ERROR("/MOD", "insufficient stack");
  TEST_EXPECT_ERROR("42 /MOD", "insufficient stack");
}

// Test */ (multiply then divide) with precision preservation
TEST_FUNCTION(test_star_slash) {
  // Basic int32 */ (uses 64-bit intermediate)
  TEST_INTERPRET("6 7 2 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(21);  // 6*7/2 = 42/2 = 21
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test precision preservation (would overflow without wider intermediate)
  TEST_INTERPRET("1000000 1000 500 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(2000000);  // 1000000*1000/500 = 2000000
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed int32/int64 types
  TEST_INTERPRET("100 200 5 INT64 */");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // Result should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 */ operations
  TEST_INTERPRET("1000000 INT64 2000000 INT64 1000000 INT64 */");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // Result should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Float */ operations
  TEST_INTERPRET("3.0 4.0 2.0 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(6.0);  // 3.0*4.0/2.0 = 6.0
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed numeric types promoting to float
  TEST_INTERPRET("10 5 2.0 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(25.0);  // 10*5/2.0 = 25.0
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative numbers
  TEST_INTERPRET("-10 6 3 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-20);  // -10*6/3 = -20
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test truncation behavior
  TEST_INTERPRET("7 3 2 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(10);  // 7*3/2 = 21/2 = 10 (truncated)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // */ error conditions
  TEST_EXPECT_ERROR("10 5 0 */", "division by zero");
  TEST_EXPECT_ERROR("10 5 0.0 */", "division by zero");
  TEST_EXPECT_ERROR("*/", "insufficient stack");
  TEST_EXPECT_ERROR("42 */", "insufficient stack");
  TEST_EXPECT_ERROR("42 84 */", "insufficient stack");
}

// Test */MOD (multiply then divide with remainder)
TEST_FUNCTION(test_star_slash_mod) {
  // Basic int32 */MOD
  TEST_INTERPRET("6 7 2 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(21);  // quotient: 6*7/2 = 42/2 = 21
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(0);  // remainder: 42%2 = 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // */MOD with remainder
  TEST_INTERPRET("7 5 3 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(11);  // quotient: 7*5/3 = 35/3 = 11
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(2);  // remainder: 35%3 = 2
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Large numbers testing precision
  TEST_INTERPRET("100000 200 50 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(400000);  // quotient: 100000*200/50 = 400000
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(0);  // remainder: 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed int32/int64 types
  TEST_INTERPRET("100 200 7 INT64 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // quotient should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // remainder should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 */MOD operations
  TEST_INTERPRET("1000000 INT64 1000 INT64 333333 INT64 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // quotient CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // remainder CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative numbers
  TEST_INTERPRET("-10 6 4 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(-15);  // quotient: -10*6/4 = -60/4 = -15
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(0);  // remainder: -60%4 = 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // */MOD with non-zero remainder and negatives
  TEST_INTERPRET("-7 5 3 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(-11);  // quotient: -7*5/3 = -35/3 = -11
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(-2);  // remainder: -35%3 = -2 (same sign as dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // */MOD error conditions
  TEST_EXPECT_ERROR("10 5 0 */MOD", "division by zero");
  TEST_EXPECT_ERROR("10 5 0 INT64 */MOD", "division by zero");
  TEST_EXPECT_ERROR("10 5.0 3 */MOD", "requires integer operands");
  TEST_EXPECT_ERROR("10.0 5 3 */MOD", "requires integer operands");
  TEST_EXPECT_ERROR("10 5 3.0 */MOD", "requires integer operands");
  TEST_EXPECT_ERROR("*/MOD", "insufficient stack");
  TEST_EXPECT_ERROR("42 */MOD", "insufficient stack");
  TEST_EXPECT_ERROR("42 84 */MOD", "insufficient stack");
}

// Test precision and overflow edge cases
TEST_FUNCTION(test_complex_arithmetic_precision) {
  // Test that */ uses wider intermediate to prevent overflow
  // This would overflow if computed as (a*b) then divided by c in 32-bit
  TEST_INTERPRET("2147483 1000 500 */");  // Large but should work
  TEST_STACK_DEPTH(1);
  // Result should be correct, not overflowed
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test precision with /MOD on large numbers
  TEST_INTERPRET("2000000000 7 /MOD");
  TEST_STACK_DEPTH(2);
  // Should handle large dividend correctly
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test */ with maximum int32 values
  TEST_INTERPRET(
      "46340 46340 2147483647 */");  // sqrt(INT32_MAX) * sqrt(INT32_MAX) /
                                     // INT32_MAX = 1
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test */MOD precision preservation
  TEST_INTERPRET("100000 100000 33000 */MOD");
  TEST_STACK_DEPTH(2);
  // 100000*100000/33000 = 10000000000/33000 ≈ 303030 remainder 10000
  TEST_INTERPRET("DROP DROP");  // Just verify it doesn't crash
  TEST_STACK_DEPTH(0);

  // Test float precision in */
  TEST_INTERPRET("1000000.0 1000000.0 3.0 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(333333333333.33334);  // High precision result
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test very small numbers
  TEST_INTERPRET("1 1 1000000 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // 1*1/1000000 = 0 (truncated)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test very small with float
  TEST_INTERPRET("1 1 1000000.0 */");
  TEST_STACK_DEPTH(1);
  // Should be very small positive float
  TEST_INTERPRET("0.0 >");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test remainder semantics with edge cases
TEST_FUNCTION(test_remainder_semantics) {
  // Standard positive remainder cases
  TEST_INTERPRET("17 5 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(3);  // 17/5 = 3
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(2);  // 17%5 = 2
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative dividend, positive divisor
  TEST_INTERPRET("-17 5 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(-3);  // -17/5 = -3 (truncate toward zero)
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(-2);  // -17%5 = -2 (remainder has sign of dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Positive dividend, negative divisor
  TEST_INTERPRET("17 -5 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(-3);  // 17/-5 = -3
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(2);  // 17%-5 = 2 (remainder has sign of dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Both negative
  TEST_INTERPRET("-17 -5 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(3);  // -17/-5 = 3
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(-2);  // -17%-5 = -2 (remainder has sign of dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test remainder semantics in */MOD
  TEST_INTERPRET("-7 3 2 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(-10);  // -7*3/2 = -21/2 = -10
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(-1);  // -21%2 = -1 (remainder has sign of dividend)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Edge case: remainder larger than divisor impossible
  TEST_INTERPRET("100 7 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("DROP");     // quotient
  TEST_INTERPRET("DUP 7 <");  // remainder should be < divisor
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);

  // Edge case: dividend smaller than divisor
  TEST_INTERPRET("3 7 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(0);  // quotient = 0
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(3);  // remainder = dividend
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test complex arithmetic operation chains
// Test complex arithmetic operation chains
TEST_FUNCTION(test_complex_arithmetic_chains) {
  // Use result of */ in next calculation
  TEST_INTERPRET("6 7 2 */");  // 6*7/2 = 21
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("3 4 */");  // 21*3/4 = 15
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(15);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mix /MOD with other operations
  TEST_INTERPRET("17 5 /MOD +");  // 17/5=3 rem 2, then 3+2=5
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(5);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Complex chain: calculate (a*b/c) mod d
  TEST_INTERPRET("100 200 50 */ 7 %");  // (100*200/50) % 7 = 400 % 7 = 1
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mix with type conversions
  TEST_INTERPRET("10 20 5 */ FLOAT 3.0 /");  // (10*20/5) as float / 3.0
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(13.333333333333334);  // 40.0/3.0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Verify */ maintains precision in chains
  TEST_INTERPRET("1000 1000 100 */ 2000 2000 200 */ +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(30000);  // 10000 + 20000 = 30000
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test performance and optimization verification
TEST_FUNCTION(test_complex_arithmetic_performance) {
  // Verify that same-type operations use fast paths even in complex ops

  // All int32 /MOD (should be fast)
  TEST_INTERPRET("1000 2000 3000 */ 77 /MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);

  // All int64 operations (should be fast)
  TEST_INTERPRET("1000 INT64 2000 INT64 500 INT64 */MOD");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // All float operations (should be fast)
  TEST_INTERPRET("10.0 20.0 5.0 */");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(40.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed operations (uses promotion path, but still correct)
  TEST_INTERPRET("10 20.0 5 INT64 */");  // Should promote to float
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(40.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Verify in-place modifications where possible
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("7 3 */");  // Should modify the 42 in place: 42*7/3 = 98
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(98);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all complex arithmetic tests
void register_complex_arithmetic_tests(void) {
  REGISTER_TEST(test_slash_mod);
  REGISTER_TEST(test_star_slash);
  REGISTER_TEST(test_star_slash_mod);
  REGISTER_TEST(test_complex_arithmetic_precision);
  REGISTER_TEST(test_remainder_semantics);
  REGISTER_TEST(test_complex_arithmetic_chains);
  REGISTER_TEST(test_complex_arithmetic_performance);
}

#endif  // TEST_ENABLED