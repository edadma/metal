#include "test_stack.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test DUP with various types and error conditions
TEST_FUNCTION(test_dup) {
  // Test DUP with int32
  TEST_INTERPRET("42");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test DUP with int64
  TEST_INTERPRET("9223372036854775807");  // Large int64
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test DUP with float
  TEST_INTERPRET("3.14159");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_FLOAT(3.14159);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_FLOAT(3.14159);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test DUP with string
  TEST_INTERPRET("\"hello\"");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test DUP with boolean
  TEST_INTERPRET("TRUE");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test DUP with array
  TEST_INTERPRET("[] 1 , 2 ,");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("DUP");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test DUP stack underflow error
  TEST_EXPECT_ERROR("DUP", "stack underflow");
}

// Test DROP with various types and error conditions
TEST_FUNCTION(test_drop) {
  // Test DROP with int32
  TEST_INTERPRET("42 84");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test DROP with float
  TEST_INTERPRET("1.5 2.5 3.5");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_FLOAT(2.5);
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test DROP with string
  TEST_INTERPRET("\"first\" \"second\"");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_STRING("first");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test DROP with mixed types
  TEST_INTERPRET("42 \"hello\" 3.14");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("DROP");  // Remove 3.14
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP");  // Remove "hello"
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test DROP stack underflow error
  TEST_EXPECT_ERROR("DROP", "stack underflow");
  // Test DROP after clearing stack
  TEST_INTERPRET("42");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
  TEST_EXPECT_ERROR("DROP", "stack underflow");
}

// Test SWAP with various types and error conditions
TEST_FUNCTION(test_swap) {
  // Test SWAP with two int32s
  TEST_INTERPRET("42 84");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("SWAP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(42);  // Was second, now on top
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(84);  // Was first, now second
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test SWAP with mixed types (int and float)
  TEST_INTERPRET("42 3.14");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("SWAP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test SWAP with string and int
  TEST_INTERPRET("\"hello\" 99");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("SWAP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(99);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test SWAP with boolean and array
  TEST_INTERPRET("TRUE [] 1 ,");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("SWAP");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP LENGTH");
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test SWAP stack underflow errors
  TEST_EXPECT_ERROR("SWAP", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("SWAP", "insufficient stack");
}

// Test PICK with various indices and error conditions
TEST_FUNCTION(test_pick) {
  // Test PICK 0 (duplicate top)
  TEST_INTERPRET("10 20 30");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("0 PICK");
  TEST_STACK_DEPTH(4);
  TEST_STACK_TOP_INT(30);  // Top duplicated
  TEST_INTERPRET("DROP DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test PICK 1 (copy second item)
  TEST_INTERPRET("10 20 30");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("1 PICK");
  TEST_STACK_DEPTH(4);
  TEST_STACK_TOP_INT(20);  // Second item copied to top
  TEST_INTERPRET("DROP DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test PICK 2 (copy third item)
  TEST_INTERPRET("10 20 30");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("2 PICK");
  TEST_STACK_DEPTH(4);
  TEST_STACK_TOP_INT(10);  // Third item copied to top
  TEST_INTERPRET("DROP DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test PICK with mixed types
  TEST_INTERPRET("\"first\" 42 3.14");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("2 PICK");  // Copy "first"
  TEST_STACK_DEPTH(4);
  TEST_STACK_TOP_STRING("first");
  TEST_INTERPRET("DROP DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test PICK with larger stack
  TEST_INTERPRET("1 2 3 4 5");
  TEST_STACK_DEPTH(5);
  TEST_INTERPRET("3 PICK");  // Copy the "2"
  TEST_STACK_DEPTH(6);
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP DROP DROP DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test PICK error conditions
  TEST_INTERPRET("10 20");
  TEST_EXPECT_ERROR("5 PICK", "insufficient stack");         // Index too large
  TEST_EXPECT_ERROR("-1 PICK", "index cannot be negative");  // Negative index
  TEST_INTERPRET("10 20");
  TEST_EXPECT_ERROR("\"hello\" PICK",
                    "index must be integer");  // Non-integer index

  // Test PICK insufficient arguments
  TEST_EXPECT_ERROR("PICK", "insufficient stack");
}

// Test ROLL with various indices and error conditions
TEST_FUNCTION(test_roll) {
  // Test ROLL 0 (no-op)
  TEST_INTERPRET("10 20 30");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("0 ROLL");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_INT(30);  // Should be unchanged
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(20);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(10);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test ROLL 1 (move second to top)
  TEST_INTERPRET("10 20 30");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("1 ROLL");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_INT(20);  // Was second, now on top
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(30);  // Was top, now second
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(10);  // Was third, unchanged
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test ROLL 2 (move third to top)
  // Test ROLL 2 (move third to top)
  TEST_INTERPRET("10 20 30");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("2 ROLL");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_INT(10);  // Third item moved to top ✓
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(30);  // Was top, now second ← FIX: should be 30, not 20
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(20);  // Was second, now third ← FIX: should be 20, not 30
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test ROLL with mixed types
  // Test ROLL with mixed types
  TEST_INTERPRET("\"first\" 42 3.14");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("2 ROLL");  // Move "first" to top
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_STRING("first");
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_FLOAT(3.14);  // ← FIX: should be 3.14, not 42
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(42);  // ← FIX: should be 42, not 3.14
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test ROLL with larger stack
  TEST_INTERPRET("1 2 3 4 5");
  TEST_STACK_DEPTH(5);
  TEST_INTERPRET("3 ROLL");  // Move "2" to top
  TEST_STACK_DEPTH(5);
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP DROP DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test ROLL error conditions
  TEST_INTERPRET("10 20");
  TEST_EXPECT_ERROR("5 ROLL", "insufficient stack");  // Index too large

  TEST_INTERPRET("10 20");
  TEST_EXPECT_ERROR("-1 ROLL", "index cannot be negative");  // Negative index

  TEST_INTERPRET("10 20");
  TEST_EXPECT_ERROR("\"hello\" ROLL",
                    "index must be integer");  // Non-integer index

  // Test ROLL insufficient arguments
  TEST_EXPECT_ERROR("ROLL", "insufficient stack");
}

// Test OVER (defined as "1 PICK")
TEST_FUNCTION(test_over) {
  // Test OVER with int32s
  TEST_INTERPRET("42 84");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("OVER");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_INT(42);  // Copy of second item
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(84);  // Original top
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(42);  // Original second
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test OVER with mixed types
  TEST_INTERPRET("\"hello\" 99");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("OVER");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test OVER with floats
  TEST_INTERPRET("1.5 2.5");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("OVER");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_FLOAT(1.5);
  TEST_INTERPRET("DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test OVER error conditions (should behave like 1 PICK)
  TEST_EXPECT_ERROR("PICK", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("PICK", "insufficient stack");
}

// Test 2DUP (defined as "OVER OVER")
TEST_FUNCTION(test_2dup) {
  // Test 2DUP with int32s
  TEST_INTERPRET("42 84");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("2DUP");
  TEST_STACK_DEPTH(4);
  TEST_STACK_TOP_INT(84);  // Copy of original top
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(42);  // Copy of original second
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(84);  // Original top
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(42);  // Original second
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 2DUP with mixed types
  TEST_INTERPRET("\"hello\" 3.14");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("2DUP");
  TEST_STACK_DEPTH(4);
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test 2DUP with arrays and booleans
  TEST_INTERPRET("[] 1 , TRUE");
  TEST_STACK_DEPTH(2);
  TEST_INTERPRET("2DUP");
  TEST_STACK_DEPTH(4);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP LENGTH");
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test 2DUP error conditions
  TEST_EXPECT_ERROR("2DUP", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("2DUP", "insufficient stack");
}

// Test ROT (defined as "2 ROLL")
TEST_FUNCTION(test_rot) {
  // Test ROT with int32s (a b c -- b c a)
  TEST_INTERPRET("10 20 30");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("ROT");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_INT(10);  // Third item moved to top
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(30);  // Top moved to second
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(20);  // Second moved to third
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test ROT with mixed types
  TEST_INTERPRET("\"first\" 42 3.14");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("ROT");
  TEST_STACK_DEPTH(3);
  TEST_STACK_TOP_STRING("first");
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test ROT with arrays and other types
  TEST_INTERPRET("[] 1 , TRUE \"hello\"");
  TEST_STACK_DEPTH(3);
  TEST_INTERPRET("ROT");
  TEST_STACK_DEPTH(3);
  // The array should now be on top, but we need to check its length
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP DROP DROP");
  TEST_STACK_DEPTH(0);

  // Test ROT error conditions (should behave like 2 ROLL)
  TEST_EXPECT_ERROR("ROT", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("ROT", "insufficient stack");
  TEST_INTERPRET("42 84");
  TEST_EXPECT_ERROR("ROT", "insufficient stack");
}

// Register all stack manipulation tests
void register_stack_tests(void) {
  REGISTER_TEST(test_dup);
  REGISTER_TEST(test_drop);
  REGISTER_TEST(test_swap);
  REGISTER_TEST(test_pick);
  REGISTER_TEST(test_roll);
  REGISTER_TEST(test_over);
  REGISTER_TEST(test_2dup);
  REGISTER_TEST(test_rot);
}

#endif  // TEST_ENABLED