#include "test_bitwise.h"

#ifdef TEST_ENABLED

#include <stdint.h>

#include "test.h"

// Test bitwise AND (&) with int32 operands
TEST_FUNCTION(test_bitwise_and_int32) {
  // Basic AND operations
  TEST_INTERPRET("5 3 &");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(1);  // 5 & 3 = 1 (binary: 101 & 011 = 001)
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // AND with zero
  TEST_INTERPRET("15 0 &");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  // AND with all bits set
  TEST_INTERPRET("255 255 &");
  TEST_STACK_TOP_INT(255);
  TEST_INTERPRET("DROP");

  // AND with negative numbers
  TEST_INTERPRET("-1 7 &");
  TEST_STACK_TOP_INT(7);  // -1 has all bits set
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise AND (&) with int64 operands
TEST_FUNCTION(test_bitwise_and_int64) {
  // Basic int64 AND
  TEST_INTERPRET("4294967296 4294967296 &");  // 2^32 & 2^32 = 2^32
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT64(4294967296LL);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // AND with large numbers
  TEST_INTERPRET("9223372036854775807 1 &");  // Max int64 & 1
  TEST_STACK_TOP_INT64(1);                    // Should be 1
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise AND (&) with mixed types (should promote to int64)
TEST_FUNCTION(test_bitwise_and_mixed_types) {
  // int32 & int64 → int64
  TEST_INTERPRET("15 4294967296 &");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT64(0);  // 15 & 2^32 = 0
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");

  // int64 & int32 → int64
  TEST_INTERPRET("4294967297 15 &");  // 2^32 + 1 & 15
  TEST_STACK_TOP_INT64(1);            // Only low bit matches
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise AND (&) error conditions
TEST_FUNCTION(test_bitwise_and_errors) {
  // Type errors with non-integers
  TEST_EXPECT_ERROR("3.14 5 &", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("5 3.14 &", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("\"hello\" 5 &",
                    "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("5 \"world\" &",
                    "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("TRUE 5 &", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("5 FALSE &", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("[] 5 &", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("NULL 5 &", "bitwise operations only work on integers");

  // Stack underflow
  TEST_EXPECT_ERROR("&", "insufficient stack");
  TEST_EXPECT_ERROR("5 &", "insufficient stack");
}

// Test bitwise OR (|) with int32 operands
TEST_FUNCTION(test_bitwise_or_int32) {
  // Basic OR operations
  TEST_INTERPRET("5 3 |");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(7);  // 5 | 3 = 7 (binary: 101 | 011 = 111)
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // OR with zero
  TEST_INTERPRET("15 0 |");
  TEST_STACK_TOP_INT(15);
  TEST_INTERPRET("DROP");

  // OR with same number
  TEST_INTERPRET("42 42 |");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  // OR with negative numbers
  TEST_INTERPRET("-1 7 |");
  TEST_STACK_TOP_INT(-1);  // -1 has all bits set
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise OR (|) with int64 operands
TEST_FUNCTION(test_bitwise_or_int64) {
  // Basic int64 OR
  TEST_INTERPRET("4294967296 1 |");  // 2^32 | 1
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT64(4294967297LL);  // 2^32 + 1
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise OR (|) with mixed types
TEST_FUNCTION(test_bitwise_or_mixed_types) {
  // int32 | int64 → int64
  TEST_INTERPRET("15 4294967296 |");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT64(4294967311LL);  // 2^32 + 15
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise OR (|) error conditions
TEST_FUNCTION(test_bitwise_or_errors) {
  // Type errors with non-integers
  TEST_EXPECT_ERROR("3.14 5 |", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("5 3.14 |", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("\"hello\" 5 |",
                    "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("TRUE 5 |", "bitwise operations only work on integers");

  // Stack underflow
  TEST_EXPECT_ERROR("|", "insufficient stack");
  TEST_EXPECT_ERROR("5 |", "insufficient stack");
}

// Test bitwise XOR (^) with int32 operands
TEST_FUNCTION(test_bitwise_xor_int32) {
  // Basic XOR operations
  TEST_INTERPRET("5 3 ^");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(6);  // 5 ^ 3 = 6 (binary: 101 ^ 011 = 110)
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // XOR with zero
  TEST_INTERPRET("15 0 ^");
  TEST_STACK_TOP_INT(15);
  TEST_INTERPRET("DROP");

  // XOR with same number (should be zero)
  TEST_INTERPRET("42 42 ^");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  // XOR with all bits set
  TEST_INTERPRET("85 255 ^");  // 85 = 01010101, 255 = 11111111
  TEST_STACK_TOP_INT(170);     // Result = 10101010 = 170
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise XOR (^) with int64 operands
TEST_FUNCTION(test_bitwise_xor_int64) {
  // Basic int64 XOR
  TEST_INTERPRET("4294967296 4294967296 ^");  // 2^32 ^ 2^32 = 0
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT64(0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise XOR (^) with mixed types
TEST_FUNCTION(test_bitwise_xor_mixed_types) {
  // int32 ^ int64 → int64
  TEST_INTERPRET("15 4294967296 ^");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT64(4294967311LL);  // 2^32 + 15
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise XOR (^) error conditions
TEST_FUNCTION(test_bitwise_xor_errors) {
  // Type errors with non-integers
  TEST_EXPECT_ERROR("3.14 5 ^", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("5 3.14 ^", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("\"hello\" 5 ^",
                    "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("FALSE 5 ^", "bitwise operations only work on integers");

  // Stack underflow
  TEST_EXPECT_ERROR("^", "insufficient stack");
  TEST_EXPECT_ERROR("5 ^", "insufficient stack");
}

// Test bitwise NOT (~) with int32 operands
TEST_FUNCTION(test_bitwise_not_int32) {
  // Basic NOT operations
  TEST_INTERPRET("0 ~");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(-1);  // ~0 = all bits set = -1
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // NOT of -1 should be 0
  TEST_INTERPRET("-1 ~");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  // NOT of positive number
  TEST_INTERPRET("42 ~");
  TEST_STACK_TOP_INT(-43);  // ~42 = -43 (two's complement)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise NOT (~) with int64 operands
TEST_FUNCTION(test_bitwise_not_int64) {
  // Basic int64 NOT
  TEST_INTERPRET("4294967296 ~");  // ~(2^32)
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test bitwise NOT (~) error conditions
TEST_FUNCTION(test_bitwise_not_errors) {
  // Type errors with non-integers
  TEST_EXPECT_ERROR("3.14 ~", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("\"hello\" ~", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("TRUE ~", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("[] ~", "bitwise operations only work on integers");
  TEST_EXPECT_ERROR("NULL ~", "bitwise operations only work on integers");

  // Stack underflow
  TEST_EXPECT_ERROR("~", "insufficient stack");
}

// Test left shift (<<) with valid operations
TEST_FUNCTION(test_left_shift_valid) {
  // Basic int32 left shift
  TEST_INTERPRET("1 1 <<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(2);  // 1 << 1 = 2
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Shift by multiple bits
  TEST_INTERPRET("5 3 <<");
  TEST_STACK_TOP_INT(40);  // 5 << 3 = 40
  TEST_INTERPRET("DROP");

  // Shift by zero
  TEST_INTERPRET("42 0 <<");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  // Basic int64 left shift
  TEST_INTERPRET("4294967296 1 <<");   // 2^32 << 1
  TEST_STACK_TOP_INT64(8589934592LL);  // 2^33
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test left shift (<<) bounds checking
TEST_FUNCTION(test_left_shift_bounds) {
  // Negative shift amount
  TEST_EXPECT_ERROR("5 -1 <<", "shift amount cannot be negative");

  // Shift amount too large for int32
  TEST_EXPECT_ERROR("5 32 <<", "shift amount too large for 32-bit value");
  TEST_EXPECT_ERROR("5 100 <<", "shift amount too large for 32-bit value");

  // Shift amount too large for int64
  TEST_EXPECT_ERROR("4294967296 64 <<",
                    "shift amount too large for 64-bit value");
  TEST_EXPECT_ERROR("4294967296 100 <<",
                    "shift amount too large for 64-bit value");

  // Boundary cases (31 and 63 should work)
  TEST_INTERPRET("1 31 <<");        // Should work for int32
  TEST_STACK_TOP_INT(-2147483648);  // 2^31 = minimum int32
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("4294967296 63 <<");  // Should work for int64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test left shift (<<) type errors
TEST_FUNCTION(test_left_shift_errors) {
  // Non-integer value
  TEST_EXPECT_ERROR("3.14 1 <<", "can only shift integers");
  TEST_EXPECT_ERROR("\"hello\" 1 <<", "can only shift integers");
  TEST_EXPECT_ERROR("TRUE 1 <<", "can only shift integers");

  // Non-int32 shift amount
  TEST_EXPECT_ERROR("5 4294967296 <<", "shift amount must be a 32-bit integer");
  TEST_EXPECT_ERROR("5 3.14 <<", "shift amount must be a 32-bit integer");
  TEST_EXPECT_ERROR("5 \"two\" <<", "shift amount must be a 32-bit integer");

  // Stack underflow
  TEST_EXPECT_ERROR("<<", "insufficient stack");
  TEST_EXPECT_ERROR("5 <<", "insufficient stack");
}

// Test arithmetic right shift (>>) with valid operations
TEST_FUNCTION(test_right_shift_valid) {
  // Basic int32 right shift
  TEST_INTERPRET("8 1 >>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(4);  // 8 >> 1 = 4
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Shift by multiple bits
  TEST_INTERPRET("40 3 >>");
  TEST_STACK_TOP_INT(5);  // 40 >> 3 = 5
  TEST_INTERPRET("DROP");

  // Shift by zero
  TEST_INTERPRET("42 0 >>");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  // Right shift negative number (sign extend)
  TEST_INTERPRET("-8 1 >>");
  TEST_STACK_TOP_INT(-4);  // Sign extends
  TEST_INTERPRET("DROP");

  // Basic int64 right shift
  TEST_INTERPRET("8589934592 1 >>");   // 2^33 >> 1
  TEST_STACK_TOP_INT64(4294967296LL);  // 2^32
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test arithmetic right shift (>>) bounds checking
TEST_FUNCTION(test_right_shift_bounds) {
  // Negative shift amount
  TEST_EXPECT_ERROR("5 -1 >>", "shift amount cannot be negative");

  // Shift amount too large for int32
  TEST_EXPECT_ERROR("5 32 >>", "shift amount too large for 32-bit value");
  TEST_EXPECT_ERROR("5 100 >>", "shift amount too large for 32-bit value");

  // Shift amount too large for int64
  TEST_EXPECT_ERROR("4294967296 64 >>",
                    "shift amount too large for 64-bit value");
  TEST_EXPECT_ERROR("4294967296 100 >>",
                    "shift amount too large for 64-bit value");

  // Boundary cases (31 and 63 should work)
  TEST_INTERPRET("-1 31 >>");  // Should work for int32
  TEST_STACK_TOP_INT(-1);      // Sign extends
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("4294967296 63 >>");  // Should work for int64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test arithmetic right shift (>>) type errors
TEST_FUNCTION(test_right_shift_errors) {
  // Non-integer value
  TEST_EXPECT_ERROR("3.14 1 >>", "can only shift integers");
  TEST_EXPECT_ERROR("\"hello\" 1 >>", "can only shift integers");
  TEST_EXPECT_ERROR("TRUE 1 >>", "can only shift integers");

  // Non-int32 shift amount
  TEST_EXPECT_ERROR("5 4294967296 >>", "shift amount must be a 32-bit integer");
  TEST_EXPECT_ERROR("5 3.14 >>", "shift amount must be a 32-bit integer");

  // Stack underflow
  TEST_EXPECT_ERROR(">>", "insufficient stack");
  TEST_EXPECT_ERROR("5 >>", "insufficient stack");
}

// Test logical right shift (>>>) with valid operations
TEST_FUNCTION(test_logical_right_shift_valid) {
  // Basic int32 logical right shift
  TEST_INTERPRET("8 1 >>>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(4);  // 8 >>> 1 = 4
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Shift by zero
  TEST_INTERPRET("42 0 >>>");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  // Logical right shift of negative number (zero fill, different from >>)
  TEST_INTERPRET("-1 1 >>>");
  TEST_STACK_TOP_INT(2147483647);  // Fills with zeros, not sign extend
  TEST_INTERPRET("DROP");

  // Basic int64 logical right shift
  TEST_INTERPRET("8589934592 1 >>>");  // 2^33 >>> 1
  TEST_STACK_TOP_INT64(4294967296LL);  // 2^32
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test logical right shift (>>>) bounds checking
TEST_FUNCTION(test_logical_right_shift_bounds) {
  // Negative shift amount
  TEST_EXPECT_ERROR("5 -1 >>>", "shift amount cannot be negative");

  // Shift amount too large for int32
  TEST_EXPECT_ERROR("5 32 >>>", "shift amount too large for 32-bit value");
  TEST_EXPECT_ERROR("5 100 >>>", "shift amount too large for 32-bit value");

  // Shift amount too large for int64
  TEST_EXPECT_ERROR("4294967296 64 >>>",
                    "shift amount too large for 64-bit value");
  TEST_EXPECT_ERROR("4294967296 100 >>>",
                    "shift amount too large for 64-bit value");

  // Boundary cases (31 and 63 should work)
  TEST_INTERPRET("-1 31 >>>");  // Should work for int32
  TEST_STACK_TOP_INT(1);        // Zero fill
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("4294967296 63 >>>");  // Should work for int64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test logical right shift (>>>) type errors
TEST_FUNCTION(test_logical_right_shift_errors) {
  // Non-integer value
  TEST_EXPECT_ERROR("3.14 1 >>>", "can only shift integers");
  TEST_EXPECT_ERROR("\"hello\" 1 >>>", "can only shift integers");
  TEST_EXPECT_ERROR("TRUE 1 >>>", "can only shift integers");

  // Non-int32 shift amount
  TEST_EXPECT_ERROR("5 4294967296 >>>",
                    "shift amount must be a 32-bit integer");
  TEST_EXPECT_ERROR("5 3.14 >>>", "shift amount must be a 32-bit integer");

  // Stack underflow
  TEST_EXPECT_ERROR(">>>", "insufficient stack");
  TEST_EXPECT_ERROR("5 >>>", "insufficient stack");
}

// Register all bitwise operation tests
void register_bitwise_tests(void) {
  REGISTER_TEST(test_bitwise_and_int32);
  REGISTER_TEST(test_bitwise_and_int64);
  REGISTER_TEST(test_bitwise_and_mixed_types);
  REGISTER_TEST(test_bitwise_and_errors);
  REGISTER_TEST(test_bitwise_or_int32);
  REGISTER_TEST(test_bitwise_or_int64);
  REGISTER_TEST(test_bitwise_or_mixed_types);
  REGISTER_TEST(test_bitwise_or_errors);
  REGISTER_TEST(test_bitwise_xor_int32);
  REGISTER_TEST(test_bitwise_xor_int64);
  REGISTER_TEST(test_bitwise_xor_mixed_types);
  REGISTER_TEST(test_bitwise_xor_errors);
  REGISTER_TEST(test_bitwise_not_int32);
  REGISTER_TEST(test_bitwise_not_int64);
  REGISTER_TEST(test_bitwise_not_errors);
  REGISTER_TEST(test_left_shift_valid);
  REGISTER_TEST(test_left_shift_bounds);
  REGISTER_TEST(test_left_shift_errors);
  REGISTER_TEST(test_right_shift_valid);
  REGISTER_TEST(test_right_shift_bounds);
  REGISTER_TEST(test_right_shift_errors);
  REGISTER_TEST(test_logical_right_shift_valid);
  REGISTER_TEST(test_logical_right_shift_bounds);
  REGISTER_TEST(test_logical_right_shift_errors);
}

#endif  // TEST_ENABLED