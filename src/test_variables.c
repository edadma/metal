#include "test_variables.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test CONSTANT creation with various types
TEST_FUNCTION(test_constant_creation_int32) {
  // Test int32 constant
  TEST_INTERPRET("42 CONSTANT test-int32-const");
  TEST_STACK_DEPTH(0);  // CONSTANT should consume the value
                        // Use the constant
  TEST_INTERPRET("test-int32-const");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Use constant in expression
  TEST_INTERPRET("test-int32-const 8 +");
  TEST_STACK_TOP_INT(50);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Use constant multiple times
  TEST_INTERPRET("test-int32-const test-int32-const *");
  TEST_STACK_TOP_INT(1764);  // 42 * 42
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_constant_creation_int64) {
  // Test int64 constant
  TEST_INTERPRET("1234567890123 CONSTANT test-int64-const");
  TEST_STACK_DEPTH(0);

  // Use the constant
  TEST_INTERPRET("test-int64-const");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Use in arithmetic
  TEST_INTERPRET("test-int64-const 1 +");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // Result should be CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_constant_creation_float) {
  // Test float constant
  TEST_INTERPRET("3.14159 CONSTANT pi");
  TEST_STACK_DEPTH(0);

  // Use the constant
  TEST_INTERPRET("pi");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(3.14159);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Use in calculation
  TEST_INTERPRET("pi 2.0 *");
  TEST_STACK_TOP_FLOAT(6.28318);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_constant_creation_string) {
  // Test string constant
  TEST_INTERPRET("\"Hello, World!\" CONSTANT greeting");
  TEST_STACK_DEPTH(0);

  // Use the constant
  TEST_INTERPRET("greeting");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_STRING("Hello, World!");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(4);  // CELL_STRING
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Use in string operation
  TEST_INTERPRET("greeting \" - from Metal\" +");
  TEST_STACK_TOP_STRING("Hello, World! - from Metal");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_constant_creation_boolean) {
  // Test boolean constants
  TEST_INTERPRET("TRUE CONSTANT always-true");
  TEST_INTERPRET("FALSE CONSTANT always-false");
  TEST_STACK_DEPTH(0);

  // Use the constants
  TEST_INTERPRET("always-true");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("always-false");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Use in logic
  TEST_INTERPRET("always-true always-false AND");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("always-true always-false OR");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_constant_creation_array) {
  // Test array constant
  TEST_INTERPRET("[] 1 , 2 , 3 , CONSTANT test-array");
  TEST_STACK_DEPTH(0);

  // Use the constant
  TEST_INTERPRET("test-array");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Access array elements through constant
  TEST_INTERPRET("test-array 1 INDEX @");
  TEST_STACK_TOP_INT(2);  // Second element (index 1)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_constant_creation_null) {
  // Test NULL constant
  TEST_INTERPRET("NULL CONSTANT null-const");
  TEST_STACK_DEPTH(0);

  // Use the constant
  TEST_INTERPRET("null-const");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(15);  // CELL_NULL
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test CONSTANT error conditions
TEST_FUNCTION(test_constant_creation_errors) {
  // Test CONSTANT with undefined value (should error)
  TEST_INTERPRET("VARIABLE temp-var");
  TEST_INTERPRET("temp-var @");  // Pushes undefined value
  TEST_EXPECT_ERROR("CONSTANT bad-const",
                    "cannot create constant with undefined value");

  // Test CONSTANT without value (stack underflow)
  TEST_EXPECT_ERROR("CONSTANT no-value-const", "insufficient stack");

  // Test CONSTANT without name (this might be handled by parser)
  TEST_EXPECT_ERROR("42 CONSTANT", "expected constant name");
}

// Test VARIABLE creation and initialization
TEST_FUNCTION(test_variable_creation) {
  // Test basic variable creation
  TEST_INTERPRET("VARIABLE test-var");
  TEST_STACK_DEPTH(0);  // VARIABLE shouldn't leave anything on stack

  // Variable should initially contain undefined
  TEST_INTERPRET("test-var @");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test that variable name puts pointer on stack
  TEST_INTERPRET("test-var");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(9);  // CELL_POINTER
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_variable_initialization_and_usage) {
  // Create and initialize variable
  TEST_INTERPRET("VARIABLE counter");
  TEST_INTERPRET("0 counter !");

  // Check initial value
  TEST_INTERPRET("counter @");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  // Modify variable
  TEST_INTERPRET("42 counter !");
  TEST_INTERPRET("counter @");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  // Use variable in expressions
  TEST_INTERPRET("counter @ 8 +");
  TEST_STACK_TOP_INT(50);
  TEST_INTERPRET("DROP");

  // Modify using +!
  TEST_INTERPRET("10 counter +!");
  TEST_INTERPRET("counter @");
  TEST_STACK_TOP_INT(52);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_variable_type_changes) {
  // Test that variables can hold different types
  TEST_INTERPRET("VARIABLE multi-type-var");

  // Store int32
  TEST_INTERPRET("42 multi-type-var !");
  TEST_INTERPRET("multi-type-var @");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");

  // Change to string
  TEST_INTERPRET("\"hello\" multi-type-var !");
  TEST_INTERPRET("multi-type-var @");
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(4);  // CELL_STRING
  TEST_INTERPRET("DROP");

  // Change to float
  TEST_INTERPRET("3.14 multi-type-var !");
  TEST_INTERPRET("multi-type-var @");
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");

  // Change to boolean
  TEST_INTERPRET("TRUE multi-type-var !");
  TEST_INTERPRET("multi-type-var @");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(3);  // CELL_BOOLEAN
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_multiple_variables) {
  // Test creating multiple variables
  TEST_INTERPRET("VARIABLE var1");
  TEST_INTERPRET("VARIABLE var2");
  TEST_INTERPRET("VARIABLE var3");

  // Initialize with different values
  TEST_INTERPRET("10 var1 !");
  TEST_INTERPRET("20 var2 !");
  TEST_INTERPRET("30 var3 !");

  // Check independence
  TEST_INTERPRET("var1 @ var2 @ var3 @ + +");
  TEST_STACK_TOP_INT(60);  // 10 + 20 + 30
  TEST_INTERPRET("DROP");

  // Modify one, others should be unchanged
  TEST_INTERPRET("100 var2 !");
  TEST_INTERPRET("var1 @");
  TEST_STACK_TOP_INT(10);  // Unchanged
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("var3 @");
  TEST_STACK_TOP_INT(30);  // Unchanged
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("var2 @");
  TEST_STACK_TOP_INT(100);  // Changed
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test VARIABLE error conditions
TEST_FUNCTION(test_variable_creation_errors) {
  // Test VARIABLE without name (parser error)
  TEST_EXPECT_ERROR("VARIABLE", "expected variable name");
}

// Test UNDEFINED? with various types
TEST_FUNCTION(test_undefined_check_with_undefined) {
  // Test with actual undefined value
  TEST_INTERPRET("VARIABLE uninit-var");
  TEST_INTERPRET("uninit-var @");  // Gets undefined
  TEST_INTERPRET("UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test multiple undefined values
  TEST_INTERPRET("VARIABLE uninit1");
  TEST_INTERPRET("VARIABLE uninit2");
  TEST_INTERPRET("uninit1 @ UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("uninit2 @ UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_undefined_check_with_defined_values) {
  // Test UNDEFINED? with int32
  TEST_INTERPRET("42 UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test UNDEFINED? with int64
  TEST_INTERPRET("1234567890123 UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test UNDEFINED? with float
  TEST_INTERPRET("3.14 UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test UNDEFINED? with string
  TEST_INTERPRET("\"hello\" UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test UNDEFINED? with boolean
  TEST_INTERPRET("TRUE UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("FALSE UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test UNDEFINED? with array
  TEST_INTERPRET("[] UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test UNDEFINED? with null
  TEST_INTERPRET("NULL UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_undefined_check_after_initialization) {
  // Test that UNDEFINED? returns false after variable initialization
  TEST_INTERPRET("VARIABLE test-init-var");

  // Before initialization
  TEST_INTERPRET("test-init-var @ UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Initialize with zero
  TEST_INTERPRET("0 test-init-var !");
  TEST_INTERPRET("test-init-var @ UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Initialize with null
  TEST_INTERPRET("NULL test-init-var !");
  TEST_INTERPRET("test-init-var @ UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Initialize with empty string
  TEST_INTERPRET("\"\" test-init-var !");
  TEST_INTERPRET("test-init-var @ UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test UNDEFINED? error conditions
TEST_FUNCTION(test_undefined_check_errors) {
  // Test UNDEFINED? stack underflow
  TEST_EXPECT_ERROR("UNDEFINED?", "insufficient stack");
}

// Test NULL constant behavior
TEST_FUNCTION(test_null_value_basic) {
  // Test NULL pushes correct value
  TEST_INTERPRET("NULL");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(15);  // CELL_NULL
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test NULL is not undefined
  TEST_INTERPRET("NULL UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test multiple NULL values
  TEST_INTERPRET("NULL NULL =");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_null_value_in_variables) {
  // Test storing NULL in variables
  TEST_INTERPRET("VARIABLE null-var");
  TEST_INTERPRET("NULL null-var !");
  TEST_INTERPRET("null-var @");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(15);  // CELL_NULL
  TEST_INTERPRET("DROP");

  // Test NULL vs undefined
  TEST_INTERPRET("null-var @ UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(false);  // NULL is not undefined
  TEST_INTERPRET("DROP");

  // Test NULL in pointer operations (should error)
  TEST_EXPECT_ERROR("null-var @ @", "not a pointer");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_null_value_comparisons) {
  // Test NULL equality
  TEST_INTERPRET("NULL NULL =");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test NULL vs other values
  TEST_INTERPRET("NULL 0 =");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("NULL \"\" =");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("NULL FALSE =");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test NULL truthiness
  TEST_INTERPRET("NULL");
  TEST_STACK_TOP_TRUTHY(false);  // NULL should be falsy
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test combinations of constants and variables
TEST_FUNCTION(test_constants_and_variables_combination) {
  // Create constant and variable with same value
  TEST_INTERPRET("42 CONSTANT the-answer");
  TEST_INTERPRET("VARIABLE answer-var");
  TEST_INTERPRET("the-answer answer-var !");

  // They should have equal values
  TEST_INTERPRET("the-answer answer-var @ =");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // But variable can be changed, constant cannot
  TEST_INTERPRET("100 answer-var !");
  TEST_INTERPRET("the-answer");
  TEST_STACK_TOP_INT(42);  // Constant unchanged
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("answer-var @");
  TEST_STACK_TOP_INT(100);  // Variable changed
  TEST_INTERPRET("DROP");

  // Use constant to reinitialize variable
  TEST_INTERPRET("the-answer answer-var !");
  TEST_INTERPRET("answer-var @");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(test_constants_in_expressions) {
  // Create some mathematical constants
  TEST_INTERPRET("3.14159 CONSTANT pi");
  TEST_INTERPRET("2.71828 CONSTANT e");

  // Use in complex expression
  TEST_INTERPRET("pi 2.0 * e +");  // 2*pi + e
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("9.0 >");  // Should be > 9
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Use constants multiple times
  TEST_INTERPRET("pi pi * e e * +");  // pi² + e²
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("17.0 >");  // Should be > 17
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test edge cases and unusual scenarios
TEST_FUNCTION(test_variables_constants_edge_cases) {
  // Test very long names (if supported)
  TEST_EXPECT_ERROR(
      "42 CONSTANT very-long-constant-name-that-tests-limits",
      "Token too long: very-long-constant-name-that-tests-limits");
  TEST_INTERPRET("42 CONSTANT veeeeeeeery-long-constant-name");
  TEST_INTERPRET("veeeeeeeery-long-constant-name");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  // Test redefining names (if allowed)
  TEST_INTERPRET("100 CONSTANT redef-test");
  TEST_INTERPRET("redef-test");
  TEST_STACK_TOP_INT(100);
  TEST_INTERPRET("DROP");

  // This might overwrite the previous definition
  TEST_INTERPRET("200 CONSTANT redef-test");
  TEST_INTERPRET("redef-test");
  TEST_STACK_TOP_INT(200);  // Should be the new value
  TEST_INTERPRET("DROP");

  // Test zero and negative constants
  TEST_INTERPRET("0 CONSTANT zero");
  TEST_INTERPRET("-42 CONSTANT negative");
  TEST_INTERPRET("zero negative +");
  TEST_STACK_TOP_INT(-42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all variables and constants tests
void register_variables_tests(void) {
  REGISTER_TEST(test_constant_creation_int32);
  REGISTER_TEST(test_constant_creation_int64);
  REGISTER_TEST(test_constant_creation_float);
  REGISTER_TEST(test_constant_creation_string);
  REGISTER_TEST(test_constant_creation_boolean);
  REGISTER_TEST(test_constant_creation_array);
  REGISTER_TEST(test_constant_creation_null);
  REGISTER_TEST(test_constant_creation_errors);
  REGISTER_TEST(test_variable_creation);
  REGISTER_TEST(test_variable_initialization_and_usage);
  REGISTER_TEST(test_variable_type_changes);
  REGISTER_TEST(test_multiple_variables);
  REGISTER_TEST(test_variable_creation_errors);
  REGISTER_TEST(test_undefined_check_with_undefined);
  REGISTER_TEST(test_undefined_check_with_defined_values);
  REGISTER_TEST(test_undefined_check_after_initialization);
  REGISTER_TEST(test_undefined_check_errors);
  REGISTER_TEST(test_null_value_basic);
  REGISTER_TEST(test_null_value_in_variables);
  REGISTER_TEST(test_null_value_comparisons);
  REGISTER_TEST(test_constants_and_variables_combination);
  REGISTER_TEST(test_constants_in_expressions);
  REGISTER_TEST(test_variables_constants_edge_cases);
}

#endif  // TEST_ENABLED