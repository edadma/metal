#include "test_conversion.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test INT32 conversion with various input types
TEST_FUNCTION(test_int32_conversion) {
  // Test INT32 with int32 (no-op)
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT32 with int64 (should truncate/convert)
  TEST_INTERPRET("1234567890123");  // Large int64
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  // Should convert to int32 (might truncate)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT32 with float
  TEST_INTERPRET("3.14");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(3);  // Should truncate to 3
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT32 with negative float
  TEST_INTERPRET("-2.7");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-2);  // Should truncate to -2
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT32 with zero
  TEST_INTERPRET("0.0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT32 error conditions
  TEST_EXPECT_ERROR("\"hello\" INT32", "cannot convert type to integer");
  TEST_EXPECT_ERROR("TRUE INT32", "cannot convert type to integer");
  TEST_EXPECT_ERROR("[] INT32", "cannot convert type to integer");
  TEST_EXPECT_ERROR("INT32", "insufficient stack");
}

// Test INT64 conversion with various input types
TEST_FUNCTION(test_int64_conversion) {
  // Test INT64 with int64 (no-op)
  TEST_INTERPRET("1234567890123");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  // Should remain int64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT64 with int32 (promotion)
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  // Should be promoted to int64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT64 with float
  TEST_INTERPRET("3.14");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  // Should truncate to int64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT64 with negative float
  TEST_INTERPRET("-99.8");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  // Should truncate to -99
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT64 with large float
  TEST_INTERPRET("1234567890.5");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  // Should truncate to 1234567890
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test INT64 error conditions
  TEST_EXPECT_ERROR("\"hello\" INT64", "cannot convert type to integer");
  TEST_EXPECT_ERROR("FALSE INT64", "cannot convert type to integer");
  TEST_EXPECT_ERROR("[] INT64", "cannot convert type to integer");
  TEST_EXPECT_ERROR("INT64", "insufficient stack");
}

// Test FLOAT conversion with various input types
TEST_FUNCTION(test_float_conversion) {
  // Test FLOAT with float (no-op)
  TEST_INTERPRET("3.14159");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(3.14159);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FLOAT with int32
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(42.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FLOAT with int64
  TEST_INTERPRET("1234567890123");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(1234567890123.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FLOAT with negative int32
  TEST_INTERPRET("-42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-42.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FLOAT with zero
  TEST_INTERPRET("0");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(0.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FLOAT error conditions
  TEST_EXPECT_ERROR("\"hello\" FLOAT", "cannot convert type to float");
  TEST_EXPECT_ERROR("TRUE FLOAT", "cannot convert type to float");
  TEST_EXPECT_ERROR("[] FLOAT", "cannot convert type to float");
  TEST_EXPECT_ERROR("FLOAT", "insufficient stack");
}

// Test type preservation - ensure conversions create correct types
TEST_FUNCTION(test_conversion_type_preservation) {
  // Test that INT32 creates actual int32 type
  TEST_INTERPRET("3.14 INT32");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");  // Should be CELL_INT32 (type 0)
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // CELL_INT32 = 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that INT64 creates actual int64 type
  TEST_INTERPRET("42 INT64");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");  // Should be CELL_INT64 (type 1)
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(1);  // CELL_INT64 = 1
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that FLOAT creates actual float type
  TEST_INTERPRET("42 FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");  // Should be CELL_FLOAT (type 2)
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT = 2
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test conversion chains - multiple conversions in sequence
TEST_FUNCTION(test_conversion_chains) {
  // Test int32 -> float -> int64
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(42.0);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  // Should be int64 now
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test float -> int32 -> float (with precision loss)
  TEST_INTERPRET("3.14159");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");  // Truncates to 3
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("FLOAT");  // Back to 3.0
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(3.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test int64 -> int32 -> int64 (potential overflow/truncation)
  TEST_INTERPRET("100");  // Small number that fits in int32
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(100);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test edge cases and boundary conditions
TEST_FUNCTION(test_conversion_edge_cases) {
  // Test conversion of maximum/minimum values
  TEST_INTERPRET("2147483647");  // INT32_MAX
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(2147483647.0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test conversion of -1
  TEST_INTERPRET("-1");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("FLOAT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(-1.0);
  TEST_INTERPRET("INT64");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test conversion of very large float to int32 (should truncate)
  TEST_INTERPRET("9999999999.9");  // Much larger than INT32_MAX
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  // Result depends on implementation, but shouldn't crash
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test conversion of very small float
  TEST_INTERPRET("0.1");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // Should truncate to 0
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test conversion of negative float to int32
  TEST_INTERPRET("-0.9");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("INT32");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);  // Should truncate toward zero
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test mixed type operations after conversion
TEST_FUNCTION(test_conversion_with_operations) {
  // Test that converted values work with arithmetic
  TEST_INTERPRET("3.7 INT32");  // Convert 3.7 to 3
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2 +");  // Add 2 to get 5
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(5);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test float conversion works with float arithmetic
  TEST_INTERPRET("5 FLOAT");  // Convert 5 to 5.0
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2.5 +");  // Add 2.5 to get 7.5
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(7.5);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that int64 conversion preserves arithmetic capability
  TEST_INTERPRET("1000000 INT64");  // Convert to int64
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("2000000 +");  // Add another large number
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all type conversion tests
void register_conversion_tests(void) {
  REGISTER_TEST(test_int32_conversion);
  REGISTER_TEST(test_int64_conversion);
  REGISTER_TEST(test_float_conversion);
  REGISTER_TEST(test_conversion_type_preservation);
  REGISTER_TEST(test_conversion_chains);
  REGISTER_TEST(test_conversion_edge_cases);
  REGISTER_TEST(test_conversion_with_operations);
}

#endif  // TEST_ENABLED
