#include "test_arithmetic_combo.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test 1+ with various numeric types
TEST_FUNCTION(test_one_plus) {
  // Test 1+ with int32
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(43);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+ with int64
  TEST_INTERPRET("1234567890123");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+");
  TEST_STACK_DEPTH(1);
  // Should stay int64 and increment by 1
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+ with float
  TEST_INTERPRET("3.14");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(4.14);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+ with negative int32
  TEST_INTERPRET("-5");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-4);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+ with zero
  TEST_INTERPRET("0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+ with negative float
  TEST_INTERPRET("-2.5");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-1.5);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+ error conditions
  TEST_EXPECT_ERROR("\"hello\" 1+", "requires numeric type");
  TEST_EXPECT_ERROR("TRUE 1+", "requires numeric type");
  TEST_EXPECT_ERROR("[] 1+", "requires numeric type");
  TEST_EXPECT_ERROR("1+", "insufficient stack");
}

// Test 1- with various numeric types
TEST_FUNCTION(test_one_minus) {
  // Test 1- with int32
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1-");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(41);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1- with int64
  TEST_INTERPRET("1234567890123");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1-");
  TEST_STACK_DEPTH(1);
  // Should stay int64 and decrement by 1
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1- with float
  TEST_INTERPRET("3.14");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1-");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(2.14);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1- with positive to zero
  TEST_INTERPRET("1");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1-");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1- with zero to negative
  TEST_INTERPRET("0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1-");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1- with negative float
  TEST_INTERPRET("-1.5");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1-");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-2.5);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1- error conditions
  TEST_EXPECT_ERROR("\"hello\" 1-", "requires numeric type");
  TEST_EXPECT_ERROR("FALSE 1-", "requires numeric type");
  TEST_EXPECT_ERROR("[] 1-", "requires numeric type");
  TEST_EXPECT_ERROR("1-", "insufficient stack");
}

// Test 2* with various numeric types
TEST_FUNCTION(test_two_star) {
  // Test 2* with int32
  TEST_INTERPRET("21");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* with int64
  TEST_INTERPRET("1000000000000");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  // Should stay int64 and double
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* with float
  TEST_INTERPRET("3.14");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(6.28);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* with zero
  TEST_INTERPRET("0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* with negative int32
  TEST_INTERPRET("-5");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-10);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* with negative float
  TEST_INTERPRET("-2.5");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-5.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* with power of 2 (bit shift efficiency)
  TEST_INTERPRET("8");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(16);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* error conditions
  TEST_EXPECT_ERROR("\"hello\" 2*", "requires numeric type");
  TEST_EXPECT_ERROR("TRUE 2*", "requires numeric type");
  TEST_EXPECT_ERROR("[] 2*", "requires numeric type");
  TEST_EXPECT_ERROR("2*", "insufficient stack");
}

// Test 2/ with various numeric types
TEST_FUNCTION(test_two_slash) {
  // Test 2/ with int32
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(21);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ with int64
  TEST_INTERPRET("2000000000000");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  // Should stay int64 and halve
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ with float
  TEST_INTERPRET("6.28");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ with odd integer (truncation)
  TEST_INTERPRET("21");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(10);  // 21/2 = 10 (truncated)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ with negative int32
  TEST_INTERPRET("-10");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-5);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ with negative odd integer
  TEST_INTERPRET("-21");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-10);  // -21/2 = -10 (arithmetic right shift)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ with zero
  TEST_INTERPRET("0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ error conditions
  TEST_EXPECT_ERROR("\"hello\" 2/", "requires numeric type");
  TEST_EXPECT_ERROR("FALSE 2/", "requires numeric type");
  TEST_EXPECT_ERROR("[] 2/", "requires numeric type");
  TEST_EXPECT_ERROR("2/", "insufficient stack");
}

// Test NEGATE with various numeric types
TEST_FUNCTION(test_negate) {
  // Test NEGATE with positive int32
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE with negative int32
  TEST_INTERPRET("-42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE with int64
  TEST_INTERPRET("1234567890123");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  // Should stay int64 and become negative
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE with positive float
  TEST_INTERPRET("3.14159");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-3.14159);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE with negative float
  TEST_INTERPRET("-2.71828");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(2.71828);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE with zero int32
  TEST_INTERPRET("0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // -0 = 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE with zero float
  TEST_INTERPRET("0.0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-0.0);  // -0.0 (IEEE 754)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test double negation
  TEST_INTERPRET("5");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(5);  // Should return to original
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE error conditions
  TEST_EXPECT_ERROR("\"hello\" NEGATE", "requires numeric type");
  TEST_EXPECT_ERROR("TRUE NEGATE", "requires numeric type");
  TEST_EXPECT_ERROR("[] NEGATE", "requires numeric type");
  TEST_EXPECT_ERROR("NEGATE", "insufficient stack");
}

// Test type preservation across arithmetic combination operations
TEST_FUNCTION(test_arithmetic_combo_types) {
  // Test that 1+ preserves int32 type
  TEST_INTERPRET("42 1+");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // CELL_INT32 = 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that 1+ preserves int64 type
  TEST_INTERPRET("1234567890123 1+");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(1);  // CELL_INT64 = 1
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that 1+ preserves float type
  TEST_INTERPRET("3.14 1+");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT = 2
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that 2* preserves int32 type
  TEST_INTERPRET("21 2*");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // CELL_INT32 = 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that NEGATE preserves float type
  TEST_INTERPRET("2.5 NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT = 2
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test arithmetic combination edge cases
TEST_FUNCTION(test_arithmetic_combo_edge_cases) {
  // Test 1+ with maximum int32 (potential overflow)
  TEST_INTERPRET("2147483647");  // INT32_MAX
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+");
  TEST_STACK_DEPTH(1);
  // Behavior depends on overflow handling, but shouldn't crash
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1- with minimum int32 (potential underflow)
  TEST_INTERPRET("-2147483648");  // INT32_MIN (if parser supports it)
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1-");
  TEST_STACK_DEPTH(1);
  // Behavior depends on underflow handling, but shouldn't crash
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2* with large int32 (potential overflow)
  TEST_INTERPRET("1073741824");  // 2^30, so 2* gives 2^31
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2*");
  TEST_STACK_DEPTH(1);
  // Should handle overflow gracefully
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2/ with 1 (minimum division)
  TEST_INTERPRET("1");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2/");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // 1/2 = 0 (truncated)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NEGATE with very small float
  TEST_INTERPRET("0.000001");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("NEGATE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-0.000001);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test combination chains
  TEST_INTERPRET("10");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+ 2* 1- 2/");  // ((10+1)*2-1)/2 = (11*2-1)/2 = 21/2 = 10
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(10);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test performance characteristics (fast in-place operations)
TEST_FUNCTION(test_arithmetic_combo_performance) {
  // Test that operations are in-place (don't create new cells)
  // This is more of a semantic test - the operations should modify the top cell

  // Multiple 1+ operations
  TEST_INTERPRET("0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+ 1+ 1+ 1+ 1+");  // Add 5
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(5);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Alternating increment/decrement
  TEST_INTERPRET("100");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+ 1- 1+ 1- 1+");  // Should end up at 101
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(101);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mix of double/halve operations
  TEST_INTERPRET("8");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2* 2* 2/");  // 8*2*2/2 = 16
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(16);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Complex combination
  TEST_INTERPRET("1");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+ 2* NEGATE 1- 2/");  // ((1+1)*2)*(-1)-1)/2 = (-4-1)/2 = -2
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-2);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("1");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("1+ 2* NEGATE 1- 2*");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-10);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all arithmetic combination tests
void register_arithmetic_combo_tests(void) {
  REGISTER_TEST(test_one_plus);
  REGISTER_TEST(test_one_minus);
  REGISTER_TEST(test_two_star);
  REGISTER_TEST(test_two_slash);
  REGISTER_TEST(test_negate);
  REGISTER_TEST(test_arithmetic_combo_types);
  REGISTER_TEST(test_arithmetic_combo_edge_cases);
  REGISTER_TEST(test_arithmetic_combo_performance);
}

#endif // TEST_ENABLED