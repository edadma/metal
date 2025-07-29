#include "test_comparison.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test = (equality) with various type combinations
TEST_FUNCTION(test_equal_comparison) {
  // int32 = int32 (fast path)
  TEST_INTERPRET("42 42 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("42 84 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 = int64 (fast path)
  TEST_INTERPRET("1234567890123 1234567890123 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float = float (fast path)
  TEST_INTERPRET("3.14 3.14 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("3.14 2.71 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed int32/int64 (promotion)
  TEST_INTERPRET("42 42 INT64 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("42 84 INT64 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed int32/float (promotion)
  TEST_INTERPRET("42 42.0 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("42 42.1 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed int64/float (promotion)
  TEST_INTERPRET("1000000000000 1000000000000.0 =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Result should be boolean type
  TEST_INTERPRET("5 5 =");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test = error conditions
  TEST_EXPECT_ERROR("=", "insufficient stack");
  TEST_EXPECT_ERROR("42 =", "insufficient stack");
}

// Test != (not equal) with various type combinations
TEST_FUNCTION(test_not_equal_comparison) {
  // int32 != int32 (fast path)
  TEST_INTERPRET("42 84 !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("42 42 !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float != float (fast path)
  TEST_INTERPRET("3.14 2.71 !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("3.14 3.14 !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed types (promotion)
  TEST_INTERPRET("42 42.1 !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("100 100 INT64 !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Result should be boolean type
  TEST_INTERPRET("5 6 !=");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Test < (less than) with various type combinations
TEST_FUNCTION(test_less_than_comparison) {
  // int32 < int32 (fast path)
  TEST_INTERPRET("5 10 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("10 5 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 5 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float < float (fast path)
  TEST_INTERPRET("2.5 3.5 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("3.5 2.5 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed int32/float (promotion)
  TEST_INTERPRET("5 5.1 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 4.9 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed int32/int64 (promotion)
  TEST_INTERPRET("100 200 INT64 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative numbers
  TEST_INTERPRET("-10 -5 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("-5 -10 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative vs positive
  TEST_INTERPRET("-5 5 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Result should be boolean type
  TEST_INTERPRET("3 7 <");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Test > (greater than) with various type combinations
TEST_FUNCTION(test_greater_than_comparison) {
  // int32 > int32 (fast path)
  TEST_INTERPRET("10 5 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 10 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 5 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float > float (fast path)
  TEST_INTERPRET("3.5 2.5 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed types (promotion)
  TEST_INTERPRET("5.1 5 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("200 INT64 100 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Zero comparisons
  TEST_INTERPRET("1 0 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0 1 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0 0 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test <= (less than or equal) with various type combinations
TEST_FUNCTION(test_less_equal_comparison) {
  // int32 <= int32 (fast path)
  TEST_INTERPRET("5 10 <=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 5 <=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("10 5 <=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float <= float (fast path)
  TEST_INTERPRET("2.5 3.5 <=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("3.5 3.5 <=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed types (promotion)
  TEST_INTERPRET("5 5.0 <=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("100 200 INT64 <=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Result should be boolean type
  TEST_INTERPRET("3 7 <=");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Test >= (greater than or equal) with various type combinations
TEST_FUNCTION(test_greater_equal_comparison) {
  // int32 >= int32 (fast path)
  TEST_INTERPRET("10 5 >=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 5 >=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 10 >=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float >= float (fast path)
  TEST_INTERPRET("3.5 2.5 >=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("3.5 3.5 >=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed types (promotion)
  TEST_INTERPRET("5.0 5 >=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("200 INT64 100 >=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test zero comparison shortcuts (0=, 0<, 0>, etc.)
TEST_FUNCTION(test_zero_comparisons) {
  // 0= tests
  TEST_INTERPRET("0 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("42 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0.0 0=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("-0.0 0=");  // IEEE 754 -0.0 should equal 0
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // 0< tests
  TEST_INTERPRET("-5 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("-3.14 0<");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // 0> tests
  TEST_INTERPRET("5 0>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("-5 0>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0 0>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // 0>= tests
  TEST_INTERPRET("5 0>=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0 0>=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("-5 0>=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // 0<= tests
  TEST_INTERPRET("-5 0<=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0 0<=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("5 0<=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // 0<> tests (not equal to zero)
  TEST_INTERPRET("5 0<>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0 0<>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("-3.14 0<>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test with boolean values
  TEST_INTERPRET("TRUE 0<>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("FALSE 0<>");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Zero comparison error conditions
  TEST_EXPECT_ERROR("0=", "insufficient stack");
  TEST_EXPECT_ERROR("0<", "insufficient stack");
  TEST_EXPECT_ERROR("[] 0<", "requires numeric");
}

// Test string comparisons
TEST_FUNCTION(test_string_comparisons) {
  // String equality
  TEST_INTERPRET("\"hello\" \"hello\" =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("\"hello\" \"world\" =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // String inequality
  TEST_INTERPRET("\"hello\" \"world\" !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("\"hello\" \"hello\" !=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // String lexicographic ordering
  TEST_INTERPRET("\"apple\" \"banana\" <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("\"banana\" \"apple\" <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("\"zebra\" \"apple\" >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // String length affects comparison
  TEST_INTERPRET("\"a\" \"aa\" <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("\"aa\" \"a\" >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Empty string comparisons
  TEST_INTERPRET("\"\" \"\" =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("\"\" \"a\" <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Case sensitivity
  TEST_INTERPRET("\"Hello\" \"hello\" =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("\"Hello\" \"hello\" <");  // 'H' < 'h' in ASCII
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test comparison error conditions
TEST_FUNCTION(test_comparison_errors) {
  // Mixed string/number comparisons (should error)
  TEST_EXPECT_ERROR("3.14 \"world\" <", "Cannot compare incompatible types");
  TEST_EXPECT_ERROR("\"test\" 100 INT64 >",
                    "Cannot compare incompatible types");

  TEST_INTERPRET("42 \"hello\" =");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("\"hello\" 42 =");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Array comparisons (not implemented)
  TEST_EXPECT_ERROR("[] [] =", "Cannot compare values of this type");
  TEST_EXPECT_ERROR("[] [] <", "Cannot compare values of this type");

  // Boolean comparisons (should work for equality only)
  TEST_INTERPRET("TRUE TRUE =");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("TRUE FALSE =");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Boolean ordering (might not be supported)
  // TEST_EXPECT_ERROR("TRUE FALSE <", "Cannot compare values of this type");

  // Stack underflow errors
  TEST_EXPECT_ERROR("=", "insufficient stack");
  TEST_EXPECT_ERROR("42 =", "insufficient stack");
  TEST_EXPECT_ERROR("<", "insufficient stack");
  TEST_EXPECT_ERROR("42 <", "insufficient stack");
  TEST_EXPECT_ERROR(">", "insufficient stack");
  TEST_EXPECT_ERROR("<=", "insufficient stack");
  TEST_EXPECT_ERROR(">=", "insufficient stack");
  TEST_EXPECT_ERROR("!=", "insufficient stack");
}

// Test comparison edge cases
TEST_FUNCTION(test_comparison_edge_cases) {
  // Very large numbers
  TEST_INTERPRET("2147483647 2147483647 =");  // INT32_MAX
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("2147483647 2147483646 >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Very small numbers (close to zero)
  TEST_INTERPRET("0.000001 0.000002 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Negative comparison edge cases
  TEST_INTERPRET("-2147483648 -2147483647 <");  // INT32_MIN vs INT32_MIN+1
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Precision edge cases with floats
  TEST_INTERPRET("1.0000001 1.0000002 <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed precision comparisons
  TEST_INTERPRET("1000000000000 1000000000000.1 <");  // int64 vs float
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Zero variants
  TEST_INTERPRET("0 -0 =");  // 0 should equal -0
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  TEST_INTERPRET("0.0 -0.0 =");  // IEEE 754: 0.0 should equal -0.0
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Boundary between int32 and int64
  TEST_INTERPRET("2147483647 2147483648 <");  // INT32_MAX < (INT32_MAX + 1)
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test comparison performance (fast paths vs promotion paths)
TEST_FUNCTION(test_comparison_performance) {
  // These should use fast paths (same types)

  // int32 comparisons (fastest)
  TEST_INTERPRET("42 84 < 100 200 < AND");  // Multiple int32 comparisons
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float comparisons (fast)
  TEST_INTERPRET("3.14 2.71 > 1.5 2.5 < AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 comparisons (fast)
  TEST_INTERPRET(
      "1000000000000 2000000000000 < 3000000000000 INT64 2000000000000 INT64 > "
      "AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // String comparisons (specialized fast path)
  TEST_INTERPRET("\"apple\" \"banana\" < \"zebra\" \"apple\" > AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Mixed type comparisons (promotion path, but still correct)
  TEST_INTERPRET("42 42.0 = 100 INT64 100 = AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Zero comparisons (should be optimized)
  TEST_INTERPRET("42 0> -5 0< AND 0 0= AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Complex comparison chains
  TEST_INTERPRET("1 2 < 2 3 < AND 3 4 < AND 4 5 < AND");  // All true
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all comparison tests
void register_comparison_tests(void) {
  REGISTER_TEST(test_equal_comparison);
  REGISTER_TEST(test_not_equal_comparison);
  REGISTER_TEST(test_less_than_comparison);
  REGISTER_TEST(test_greater_than_comparison);
  REGISTER_TEST(test_less_equal_comparison);
  REGISTER_TEST(test_greater_equal_comparison);
  REGISTER_TEST(test_zero_comparisons);
  REGISTER_TEST(test_string_comparisons);
  REGISTER_TEST(test_comparison_errors);
  REGISTER_TEST(test_comparison_edge_cases);
  REGISTER_TEST(test_comparison_performance);
}

#endif  // TEST_ENABLED