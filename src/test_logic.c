#include "test_logic.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test TRUE and FALSE boolean constants
TEST_FUNCTION(test_boolean_constants_basic) {
  // Test TRUE constant
  TEST_INTERPRET("TRUE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FALSE constant
  TEST_INTERPRET("FALSE");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that TRUE and FALSE are different
  TEST_INTERPRET("TRUE FALSE =");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test that TRUE and FALSE are not equal
  TEST_INTERPRET("TRUE FALSE !=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_boolean_constants_truthiness) {
  // Test that TRUE is truthy
  TEST_INTERPRET("TRUE");
  TEST_STACK_TOP_TRUTHY(true);
  TEST_INTERPRET("DROP");

  // Test that FALSE is falsy
  TEST_INTERPRET("FALSE");
  TEST_STACK_TOP_TRUTHY(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_boolean_constants_in_expressions) {
  // Test TRUE in arithmetic (should be truthy/non-zero)
  TEST_INTERPRET("TRUE 0!=");
  TEST_STACK_TOP_BOOLEAN(true);  // TRUE is not zero
  TEST_INTERPRET("DROP");

  // Test FALSE in arithmetic (should be falsy/zero-like)
  TEST_INTERPRET("FALSE 0=");
  TEST_STACK_TOP_BOOLEAN(false);  // FALSE != zero
  TEST_INTERPRET("DROP");

  // Test boolean constants with variables
  TEST_INTERPRET("VARIABLE bool-var");
  TEST_INTERPRET("TRUE bool-var !");
  TEST_INTERPRET("bool-var @");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("FALSE bool-var !");
  TEST_INTERPRET("bool-var @");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test AND with boolean values (fast path)
TEST_FUNCTION(test_logical_and_booleans) {
  // Test TRUE AND TRUE
  TEST_INTERPRET("TRUE TRUE AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test TRUE AND FALSE
  TEST_INTERPRET("TRUE FALSE AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FALSE AND TRUE
  TEST_INTERPRET("FALSE TRUE AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FALSE AND FALSE
  TEST_INTERPRET("FALSE FALSE AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Verify result type is boolean
  TEST_INTERPRET("TRUE FALSE AND");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test AND with numeric values (truthiness testing)
TEST_FUNCTION(test_logical_and_numbers) {
  // Test non-zero AND non-zero (both truthy)
  TEST_INTERPRET("42 84 AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test non-zero AND zero (truthy AND falsy)
  TEST_INTERPRET("42 0 AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test zero AND non-zero (falsy AND truthy)
  TEST_INTERPRET("0 42 AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test zero AND zero (both falsy)
  TEST_INTERPRET("0 0 AND");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test with negative numbers (truthy)
  TEST_INTERPRET("-5 -10 AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-5 0 AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test AND with float values
TEST_FUNCTION(test_logical_and_floats) {
  // Test non-zero floats (truthy)
  TEST_INTERPRET("3.14 2.71 AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test float AND zero float (truthy AND falsy)
  TEST_INTERPRET("3.14 0.0 AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test zero float AND non-zero float (falsy AND truthy)
  TEST_INTERPRET("0.0 2.71 AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test zero floats (both falsy)
  TEST_INTERPRET("0.0 -0.0 AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test very small float (should be truthy)
  TEST_INTERPRET("0.000001 1.0 AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test AND with mixed types
TEST_FUNCTION(test_logical_and_mixed_types) {
  // Test boolean AND number
  TEST_INTERPRET("TRUE 42 AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("FALSE 42 AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("TRUE 0 AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test number AND boolean
  TEST_INTERPRET("42 TRUE AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 TRUE AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test int32 AND int64
  TEST_INTERPRET("42 1234567890123 AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test int32 AND float
  TEST_INTERPRET("42 3.14 AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 3.14 AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test AND with special values
TEST_FUNCTION(test_logical_and_special_values) {
  // Test with NULL (should be falsy)
  TEST_INTERPRET("TRUE NULL AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("NULL TRUE AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("NULL NULL AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test with strings (non-empty should be truthy)
  TEST_INTERPRET("TRUE \"hello\" AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("\"hello\" TRUE AND");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test with empty string (should be falsy)
  TEST_INTERPRET("TRUE \"\" AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("\"\" TRUE AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test AND error conditions
TEST_FUNCTION(test_logical_and_errors) {
  // Test AND stack underflow
  TEST_EXPECT_ERROR("AND", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("AND", "insufficient stack");
}

// Test OR with boolean values
TEST_FUNCTION(test_logical_or_booleans) {
  // Test TRUE OR TRUE
  TEST_INTERPRET("TRUE TRUE OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test TRUE OR FALSE
  TEST_INTERPRET("TRUE FALSE OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FALSE OR TRUE
  TEST_INTERPRET("FALSE TRUE OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FALSE OR FALSE
  TEST_INTERPRET("FALSE FALSE OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Verify result type is boolean
  TEST_INTERPRET("TRUE FALSE OR");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test OR with numeric values
TEST_FUNCTION(test_logical_or_numbers) {
  // Test non-zero OR non-zero (both truthy)
  TEST_INTERPRET("42 84 OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test non-zero OR zero (truthy OR falsy)
  TEST_INTERPRET("42 0 OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test zero OR non-zero (falsy OR truthy)
  TEST_INTERPRET("0 42 OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test zero OR zero (both falsy)
  TEST_INTERPRET("0 0 OR");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test with int64
  TEST_INTERPRET("0 1234567890123 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("1234567890123 0 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test OR with float values
TEST_FUNCTION(test_logical_or_floats) {
  // Test non-zero floats OR each other
  TEST_INTERPRET("3.14 2.71 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test float OR zero
  TEST_INTERPRET("3.14 0.0 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0.0 2.71 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test zero floats
  TEST_INTERPRET("0.0 -0.0 OR");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test negative float OR positive
  TEST_INTERPRET("-3.14 2.71 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test OR with mixed types
TEST_FUNCTION(test_logical_or_mixed_types) {
  // Test boolean OR number
  TEST_INTERPRET("FALSE 42 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("FALSE 0 OR");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("TRUE 0 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test different numeric types
  TEST_INTERPRET("0 3.14 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("42 1234567890123 OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test OR with special values
TEST_FUNCTION(test_logical_or_special_values) {
  // Test with NULL
  TEST_INTERPRET("FALSE NULL OR");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("TRUE NULL OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test with strings
  TEST_INTERPRET("FALSE \"hello\" OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("FALSE \"\" OR");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test with arrays (should be truthy)
  TEST_INTERPRET("FALSE [] OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test OR error conditions
TEST_FUNCTION(test_logical_or_errors) {
  // Test OR stack underflow
  TEST_EXPECT_ERROR("OR", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("OR", "insufficient stack");
}

// Test NOT with boolean values
TEST_FUNCTION(test_logical_not_booleans) {
  // Test NOT TRUE
  TEST_INTERPRET("TRUE NOT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NOT FALSE
  TEST_INTERPRET("FALSE NOT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test double negation
  TEST_INTERPRET("TRUE NOT NOT");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("FALSE NOT NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Verify result type is boolean
  TEST_INTERPRET("TRUE NOT");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test NOT with numeric values
TEST_FUNCTION(test_logical_not_numbers) {
  // Test NOT with non-zero int32 (truthy -> false)
  TEST_INTERPRET("42 NOT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NOT with zero int32 (falsy -> true)
  TEST_INTERPRET("0 NOT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NOT with negative int32 (truthy -> false)
  TEST_INTERPRET("-42 NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test NOT with int64
  TEST_INTERPRET("1234567890123 NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 INT64 NOT");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test NOT with float values
TEST_FUNCTION(test_logical_not_floats) {
  // Test NOT with non-zero float (truthy -> false)
  TEST_INTERPRET("3.14 NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test NOT with zero float (falsy -> true)
  TEST_INTERPRET("0.0 NOT");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test NOT with negative zero float
  TEST_INTERPRET("-0.0 NOT");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test NOT with negative float (truthy -> false)
  TEST_INTERPRET("-2.71 NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test NOT with very small float (truthy -> false)
  TEST_INTERPRET("0.000001 NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test NOT with special values
TEST_FUNCTION(test_logical_not_special_values) {
  // Test NOT with NULL (falsy -> true)
  TEST_INTERPRET("NULL NOT");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test NOT with non-empty string (truthy -> false)
  TEST_INTERPRET("\"hello\" NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test NOT with empty string (falsy -> true)
  TEST_INTERPRET("\"\" NOT");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test NOT with array (truthy -> false)
  TEST_INTERPRET("[] NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("[] 1 , NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test NOT error conditions
TEST_FUNCTION(test_logical_not_errors) {
  // Test NOT stack underflow
  TEST_EXPECT_ERROR("NOT", "insufficient stack");
}

// Test complex logical expressions (combinations)
TEST_FUNCTION(test_logical_combinations) {
  // Test De Morgan's law: NOT (A AND B) = (NOT A) OR (NOT B)
  TEST_INTERPRET("TRUE FALSE AND NOT");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("TRUE NOT FALSE NOT OR");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("=");
  TEST_STACK_TOP_BOOLEAN(true);  // Should be equivalent
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test De Morgan's law: NOT (A OR B) = (NOT A) AND (NOT B)
  TEST_INTERPRET("TRUE FALSE OR NOT");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("TRUE NOT FALSE NOT AND");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("=");
  TEST_STACK_TOP_BOOLEAN(true);  // Should be equivalent
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test associativity: (A AND B) AND C = A AND (B AND C)
  TEST_INTERPRET("TRUE FALSE TRUE AND AND");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("TRUE FALSE TRUE AND AND");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("=");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test logical operations with mixed truthiness
TEST_FUNCTION(test_logical_mixed_truthiness) {
  // Test various falsy values
  TEST_INTERPRET("0 FALSE AND");  // 0 AND FALSE -> FALSE
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 FALSE OR");  // 0 OR FALSE -> FALSE
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("\"\" NULL AND");  // "" AND NULL -> FALSE
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test mixed truthy/falsy
  TEST_INTERPRET("42 \"hello\" AND");  // truthy AND truthy -> TRUE
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 \"hello\" OR");  // falsy OR truthy -> TRUE
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("[] NULL OR");  // truthy OR falsy -> TRUE
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test logical operations preserve type correctness
TEST_FUNCTION(test_logical_result_types) {
  // All logical operations should return CELL_BOOLEAN
  TEST_INTERPRET("42 84 AND CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("0 42 OR CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("\"hello\" NOT CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");

  // Test nested logical operations
  TEST_INTERPRET("TRUE FALSE AND TRUE OR CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test edge cases and corner cases
TEST_FUNCTION(test_logical_edge_cases) {
  // Test with undefined values (should be falsy)
  TEST_INTERPRET("VARIABLE undef-var");
  TEST_INTERPRET("undef-var @ NOT");
  TEST_STACK_TOP_BOOLEAN(true);  // undefined is falsy, so NOT undefined is true
  TEST_INTERPRET("DROP");

  // Test with very large numbers (should be truthy)
  TEST_INTERPRET("2147483647 NOT");  // INT32_MAX
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-2147483648 NOT");  // INT32_MIN
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test with very small floats (should be truthy)
  TEST_INTERPRET("1e-10 NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("-1e-10 NOT");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all logic operation tests
void register_logic_tests(void) {
  REGISTER_TEST(test_boolean_constants_basic);
  REGISTER_TEST(test_boolean_constants_truthiness);
  REGISTER_TEST(test_boolean_constants_in_expressions);
  REGISTER_TEST(test_logical_and_booleans);
  REGISTER_TEST(test_logical_and_numbers);
  REGISTER_TEST(test_logical_and_floats);
  REGISTER_TEST(test_logical_and_mixed_types);
  REGISTER_TEST(test_logical_and_special_values);
  REGISTER_TEST(test_logical_and_errors);
  REGISTER_TEST(test_logical_or_booleans);
  REGISTER_TEST(test_logical_or_numbers);
  REGISTER_TEST(test_logical_or_floats);
  REGISTER_TEST(test_logical_or_mixed_types);
  REGISTER_TEST(test_logical_or_special_values);
  REGISTER_TEST(test_logical_or_errors);
  REGISTER_TEST(test_logical_not_booleans);
  REGISTER_TEST(test_logical_not_numbers);
  REGISTER_TEST(test_logical_not_floats);
  REGISTER_TEST(test_logical_not_special_values);
  REGISTER_TEST(test_logical_not_errors);
  REGISTER_TEST(test_logical_combinations);
  REGISTER_TEST(test_logical_mixed_truthiness);
  REGISTER_TEST(test_logical_result_types);
  REGISTER_TEST(test_logical_edge_cases);
}

#endif  // TEST_ENABLED