#include "test_string.h"

#ifdef TEST_ENABLED
#include "stack.h"
#include "strings.h"
#include "test.h"

// Test basic internment behavior - compilation vs interpretation
TEST_FUNCTION(test_string_internment_basic) {
  // Test that string literals in definitions get interned
  TEST_INTERPRET("DEF test-interned-word \"hello\" END");
  TEST_INTERPRET("test-interned-word STRING-INTERNED?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test that string literals in interpretation mode are allocated
  TEST_INTERPRET("\"hello\" STRING-INTERNED?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  // Test that compiled strings have correct flags
  TEST_INTERPRET("test-interned-word CELL-FLAGS");
  TEST_STACK_DEPTH(1);
  // CELL_FLAG_INTERNED = 2, so should have bit 1 set
  TEST_INTERPRET("2 &");
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");
  // Test that allocated strings don't have interned flag
  TEST_INTERPRET("\"world\" CELL-FLAGS 2 &");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
}

// Test that same content strings share pointers when interned
TEST_FUNCTION(test_string_identity_sharing) {
  // Test that multiple definitions with same string content share pointers
  TEST_INTERPRET("DEF word1 \"shared\" END");
  TEST_INTERPRET("DEF word2 \"shared\" END");
  TEST_INTERPRET("word1 word2 STRING-SAME-PTR?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test that different content has different pointers
  TEST_INTERPRET("DEF word3 \"different\" END");
  TEST_INTERPRET("word1 word3 STRING-SAME-PTR?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  // Test that allocated strings with same content don't share pointers
  TEST_INTERPRET("\"allocated1\" \"allocated1\" STRING-SAME-PTR?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
}

// Test that interned and allocated strings with same content are equal
TEST_FUNCTION(test_string_cross_type_equality) {
  // Create an interned string
  TEST_INTERPRET("DEF test-equal-word \"crosstype\" END");
  // Test equality between interned and allocated
  TEST_INTERPRET("test-equal-word \"crosstype\" =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test inequality with different content
  TEST_INTERPRET("test-equal-word \"different\" =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  // Test that comparison operators work across types
  TEST_INTERPRET("test-equal-word \"aaa\" >");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("test-equal-word \"zzz\" <");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
}

// Test that string operations work identically for interned and allocated
TEST_FUNCTION(test_string_operations_identical) {
  // Create interned and allocated versions of same string
  TEST_INTERPRET("DEF test-ops-word \"operations\" END");
  // Test that length is the same
  TEST_INTERPRET("test-ops-word LENGTH");
  TEST_INTERPRET("\"operations\" LENGTH");
  TEST_INTERPRET("=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test that empty check works the same
  TEST_INTERPRET("test-ops-word LENGTH 0 =");
  TEST_INTERPRET("\"operations\" LENGTH 0 =");
  TEST_INTERPRET("=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test with empty strings
  TEST_INTERPRET("DEF empty-word \"\" END");
  TEST_INTERPRET("empty-word LENGTH 0 =");
  TEST_INTERPRET("\"\" LENGTH 0 =");
  TEST_INTERPRET("=");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
}

// Test edge cases for string internment
TEST_FUNCTION(test_string_internment_edge_cases) {
  // Test empty string internment
  TEST_INTERPRET("DEF empty-interned \"\" END");
  TEST_INTERPRET("empty-interned STRING-INTERNED?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test that multiple empty strings share pointers
  TEST_INTERPRET("DEF empty1 \"\" END");
  TEST_INTERPRET("DEF empty2 \"\" END");
  TEST_INTERPRET("empty1 empty2 STRING-SAME-PTR?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test strings with special characters
  TEST_INTERPRET("DEF special-chars \"hello\\nworld\\t!\" END");
  TEST_INTERPRET("special-chars STRING-INTERNED?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test that special char strings are equal across types
  TEST_INTERPRET("special-chars \"hello\\nworld\\t!\" =");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  // Test longer strings (if MAX_TOKEN_SIZE allows)
  TEST_INTERPRET("DEF longer-string \"This is a somewhat longer string to test internment\" END");
  TEST_INTERPRET("longer-string STRING-INTERNED?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
}

// Test intern count tracking
TEST_FUNCTION(test_intern_count_tracking) {
  // Get initial count
  TEST_INTERPRET("INTERN-COUNT");
  int initial_count = data_peek_cell(&test_context, 0).payload.i32;
  TEST_INTERPRET("DROP");
  // Add one interned string
  TEST_INTERPRET("DEF count-test \"unique-string-for-counting\" END");
  TEST_INTERPRET("INTERN-COUNT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(initial_count + 1);
  TEST_INTERPRET("DROP");
  // Add another with same content - count shouldn't increase
  TEST_INTERPRET("DEF count-test2 \"unique-string-for-counting\" END");
  TEST_INTERPRET("INTERN-COUNT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(initial_count + 1);  // Same count
  TEST_INTERPRET("DROP");

  // Add different content - count should increase
  TEST_INTERPRET("DEF count-test3 \"different-unique-string\" END");
  TEST_INTERPRET("INTERN-COUNT");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(initial_count + 2);
  TEST_INTERPRET("DROP");

  // Verify the duplicate strings share pointers
  TEST_INTERPRET("count-test count-test2 STRING-SAME-PTR?");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
}

// Register all string tests
void register_string_tests(void) {
  REGISTER_TEST(test_string_internment_basic);
  REGISTER_TEST(test_string_identity_sharing);
  REGISTER_TEST(test_string_cross_type_equality);
  REGISTER_TEST(test_string_operations_identical);
  REGISTER_TEST(test_string_internment_edge_cases);
  REGISTER_TEST(test_intern_count_tracking);
}

#endif  // TEST_ENABLED