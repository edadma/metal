#include "test_memory_ops.h"

#ifdef TEST_ENABLED

#include "test.h"

// Test @ (fetch) with various types and valid pointers
TEST_FUNCTION(test_fetch_valid_pointers) {
  // Test fetch from int32 variable
  TEST_INTERPRET("VARIABLE int-var");
  TEST_INTERPRET("42 int-var !");
  TEST_INTERPRET("int-var @");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test fetch from int64 variable
  TEST_INTERPRET("VARIABLE int64-var");
  TEST_INTERPRET("1234567890123 int64-var !");
  TEST_INTERPRET("int64-var @");
  TEST_STACK_DEPTH(1);
  // Should be int64 value
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test fetch from float variable
  TEST_INTERPRET("VARIABLE float-var");
  TEST_INTERPRET("3.14159 float-var !");
  TEST_INTERPRET("float-var @");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_FLOAT(3.14159);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test fetch from string variable
  TEST_INTERPRET("VARIABLE string-var");
  TEST_INTERPRET("\"hello world\" string-var !");
  TEST_INTERPRET("string-var @");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_STRING("hello world");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test fetch from boolean variable
  TEST_INTERPRET("VARIABLE bool-var");
  TEST_INTERPRET("TRUE bool-var !");
  TEST_INTERPRET("bool-var @");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test fetch from array variable
  TEST_INTERPRET("VARIABLE array-var");
  TEST_INTERPRET("[] 1 , 2 , 3 , array-var !");
  TEST_INTERPRET("array-var @");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test @ (fetch) error conditions
TEST_FUNCTION(test_fetch_errors) {
  // Test fetch with null pointer
  TEST_EXPECT_ERROR("NULL @", "not a pointer");

  // Test fetch with non-pointer type
  TEST_EXPECT_ERROR("42 @", "not a pointer");
  TEST_EXPECT_ERROR("\"hello\" @", "not a pointer");
  TEST_EXPECT_ERROR("3.14 @", "not a pointer");
  TEST_EXPECT_ERROR("TRUE @", "not a pointer");
  TEST_EXPECT_ERROR("[] @", "not a pointer");

  // Test fetch stack underflow
  TEST_EXPECT_ERROR("@", "stack underflow");
}

// Test fetch from undefined variable
TEST_FUNCTION(test_fetch_undefined) {
  // Test fetch from uninitialized variable (should return undefined)
  TEST_INTERPRET("VARIABLE uninit-var");
  TEST_INTERPRET("uninit-var @");
  TEST_STACK_DEPTH(1);
  TEST_INTERPRET("UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test ! (store) with various types
TEST_FUNCTION(test_store_valid_operations) {
  // Test store int32
  TEST_INTERPRET("VARIABLE test-var");
  TEST_INTERPRET("42 test-var !");
  TEST_INTERPRET("test-var @");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test store int64
  TEST_INTERPRET("VARIABLE test-var2");
  TEST_INTERPRET("9876543210987 test-var2 !");
  TEST_INTERPRET("test-var2 @");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test store float
  TEST_INTERPRET("VARIABLE test-var3");
  TEST_INTERPRET("2.71828 test-var3 !");
  TEST_INTERPRET("test-var3 @");
  TEST_STACK_TOP_FLOAT(2.71828);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test store string
  TEST_INTERPRET("VARIABLE test-var4");
  TEST_INTERPRET("\"testing\" test-var4 !");
  TEST_INTERPRET("test-var4 @");
  TEST_STACK_TOP_STRING("testing");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test store boolean
  TEST_INTERPRET("VARIABLE test-var5");
  TEST_INTERPRET("FALSE test-var5 !");
  TEST_INTERPRET("test-var5 @");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test overwriting existing values
  TEST_INTERPRET("VARIABLE overwrite-var");
  TEST_INTERPRET("100 overwrite-var !");
  TEST_INTERPRET("overwrite-var @");
  TEST_STACK_TOP_INT(100);
  TEST_INTERPRET("DROP");
  // Overwrite with different type
  TEST_INTERPRET("\"new value\" overwrite-var !");
  TEST_INTERPRET("overwrite-var @");
  TEST_STACK_TOP_STRING("new value");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test ! (store) error conditions
TEST_FUNCTION(test_store_errors) {
  // Test store to null pointer
  TEST_EXPECT_ERROR("42 NULL !", "not a pointer");

  // Test store to non-pointer types
  TEST_EXPECT_ERROR("42 84 !", "not a pointer");
  TEST_EXPECT_ERROR("42 \"hello\" !", "not a pointer");
  TEST_EXPECT_ERROR("42 3.14 !", "not a pointer");
  TEST_EXPECT_ERROR("42 TRUE !", "not a pointer");
  TEST_EXPECT_ERROR("42 [] !", "not a pointer");

  // Test store undefined value (should error)
  TEST_INTERPRET("VARIABLE temp-var");
  TEST_INTERPRET("temp-var @");  // Gets undefined value
  TEST_INTERPRET("VARIABLE target-var");
  TEST_EXPECT_ERROR("target-var !", "cannot store undefined value");

  // Test store stack underflow
  TEST_EXPECT_ERROR("!", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("!", "insufficient stack");
}

// Test +! (plus store) with type promotion
TEST_FUNCTION(test_plus_store_type_promotion) {
  // int32 += int32 (same type, fast path)
  TEST_INTERPRET("VARIABLE var1");
  TEST_INTERPRET("10 var1 !");
  TEST_INTERPRET("5 var1 +!");
  TEST_INTERPRET("var1 @");
  TEST_STACK_TOP_INT(15);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 += int64 (promotion to int64)
  TEST_INTERPRET("VARIABLE var2");
  TEST_INTERPRET("10 var2 !");
  TEST_INTERPRET("1000000000000 var2 +!");
  TEST_INTERPRET("var2 @");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64 (promoted)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int32 += float (promotion to float)
  TEST_INTERPRET("VARIABLE var3");
  TEST_INTERPRET("10 var3 !");
  TEST_INTERPRET("2.5 var3 +!");
  TEST_INTERPRET("var3 @");
  TEST_STACK_TOP_FLOAT(12.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT (promoted)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 += int32 (stays int64)
  TEST_INTERPRET("VARIABLE var4");
  TEST_INTERPRET("2000000000000 var4 !");
  TEST_INTERPRET("42 var4 +!");
  TEST_INTERPRET("var4 @");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // int64 += float (promotion to float)
  TEST_INTERPRET("VARIABLE var5");
  TEST_INTERPRET("1000000000000 var5 !");
  TEST_INTERPRET("0.5 var5 +!");
  TEST_INTERPRET("var5 @");
  TEST_STACK_TOP_FLOAT(1000000000000.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT (promoted)
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float += int32 (stays float)
  TEST_INTERPRET("VARIABLE var6");
  TEST_INTERPRET("3.14 var6 !");
  TEST_INTERPRET("2 var6 +!");
  TEST_INTERPRET("var6 @");
  TEST_STACK_TOP_FLOAT(5.14);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // float += float (stays float)
  TEST_INTERPRET("VARIABLE var7");
  TEST_INTERPRET("1.5 var7 !");
  TEST_INTERPRET("2.5 var7 +!");
  TEST_INTERPRET("var7 @");
  TEST_STACK_TOP_FLOAT(4.0);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test +! (plus store) error conditions
TEST_FUNCTION(test_plus_store_errors) {
  // Test +! with null pointer
  TEST_EXPECT_ERROR("5 NULL +!", "cannot use null value as pointer");

  // Test +! with non-pointer address
  TEST_EXPECT_ERROR("5 42 +!", "second argument must be a pointer");
  TEST_EXPECT_ERROR("5 \"hello\" +!", "second argument must be a pointer");
  TEST_EXPECT_ERROR("5 3.14 +!", "second argument must be a pointer");

  // Test +! with null pointer stored in variable
  TEST_INTERPRET("VARIABLE null-var");
  TEST_INTERPRET("NULL null-var !");
  TEST_EXPECT_ERROR("5 null-var @ +!", "cannot use null value as pointer");

  // Test +! with non-numeric target
  TEST_INTERPRET("VARIABLE string-var");
  TEST_INTERPRET("\"hello\" string-var !");
  TEST_EXPECT_ERROR("5 string-var +!", "target must be numeric");

  TEST_INTERPRET("VARIABLE bool-var");
  TEST_INTERPRET("TRUE bool-var !");
  TEST_EXPECT_ERROR("5 bool-var +!", "target must be numeric");

  // Test +! with non-numeric value
  TEST_INTERPRET("VARIABLE num-var");
  TEST_INTERPRET("10 num-var !");
  TEST_EXPECT_ERROR("\"hello\" num-var +!", "incompatible types");
  TEST_EXPECT_ERROR("TRUE num-var +!", "incompatible types");
  TEST_EXPECT_ERROR("[] num-var +!", "incompatible types");

  // Test +! stack underflow
  TEST_EXPECT_ERROR("+!", "insufficient stack");
  TEST_INTERPRET("42");
  TEST_EXPECT_ERROR("+!", "insufficient stack");
}

// Test 1+! (one plus store) with various numeric types
TEST_FUNCTION(test_one_plus_store) {
  // Test 1+! with int32
  TEST_INTERPRET("VARIABLE int32-var");
  TEST_INTERPRET("41 int32-var !");
  TEST_INTERPRET("int32-var 1+!");
  TEST_INTERPRET("int32-var @");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+! with int64
  TEST_INTERPRET("VARIABLE int64-var");
  TEST_INTERPRET("999999999999 int64-var !");
  TEST_INTERPRET("int64-var 1+!");
  TEST_INTERPRET("int64-var @");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+! with float
  TEST_INTERPRET("VARIABLE float-var");
  TEST_INTERPRET("2.5 float-var !");
  TEST_INTERPRET("float-var 1+!");
  TEST_INTERPRET("float-var @");
  TEST_STACK_TOP_FLOAT(3.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test multiple 1+! operations
  TEST_INTERPRET("VARIABLE counter");
  TEST_INTERPRET("0 counter !");
  TEST_INTERPRET("counter 1+!");
  TEST_INTERPRET("counter 1+!");
  TEST_INTERPRET("counter 1+!");
  TEST_INTERPRET("counter @");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+! with negative numbers
  TEST_INTERPRET("VARIABLE neg-var");
  TEST_INTERPRET("-5 neg-var !");
  TEST_INTERPRET("neg-var 1+!");
  TEST_INTERPRET("neg-var @");
  TEST_STACK_TOP_INT(-4);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 1+! (one plus store) error conditions
TEST_FUNCTION(test_one_plus_store_errors) {
  // Test 1+! with null pointer
  TEST_EXPECT_ERROR("NULL 1+!", "argument must be a pointer");

  // Test 1+! with non-pointer types
  TEST_EXPECT_ERROR("42 1+!", "argument must be a pointer");
  TEST_EXPECT_ERROR("\"hello\" 1+!", "argument must be a pointer");
  TEST_EXPECT_ERROR("3.14 1+!", "argument must be a pointer");
  TEST_EXPECT_ERROR("TRUE 1+!", "argument must be a pointer");

  // Test 1+! with null pointer stored in variable
  TEST_INTERPRET("VARIABLE null-ptr-var");
  TEST_INTERPRET("NULL null-ptr-var !");
  TEST_EXPECT_ERROR("null-ptr-var @ 1+!", "argument must be a pointer");

  // Test 1+! with non-numeric target
  TEST_INTERPRET("VARIABLE string-target");
  TEST_INTERPRET("\"hello\" string-target !");
  TEST_EXPECT_ERROR("string-target 1+!", "target must be numeric");

  TEST_INTERPRET("VARIABLE bool-target");
  TEST_INTERPRET("FALSE bool-target !");
  TEST_EXPECT_ERROR("bool-target 1+!", "target must be numeric");

  TEST_INTERPRET("VARIABLE array-target");
  TEST_INTERPRET("[] array-target !");
  TEST_EXPECT_ERROR("array-target 1+!", "target must be numeric");

  // Test 1+! stack underflow
  TEST_EXPECT_ERROR("1+!", "insufficient stack");
}

// Test 1-! (one minus store) with various numeric types
TEST_FUNCTION(test_one_minus_store) {
  // Test 1-! with int32
  TEST_INTERPRET("VARIABLE int32-var");
  TEST_INTERPRET("43 int32-var !");
  TEST_INTERPRET("int32-var 1-!");
  TEST_INTERPRET("int32-var @");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(0);  // CELL_INT32
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1-! with int64
  TEST_INTERPRET("VARIABLE int64-var");
  TEST_INTERPRET("1000000000001 int64-var !");
  TEST_INTERPRET("int64-var 1-!");
  TEST_INTERPRET("int64-var @");
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(1);  // CELL_INT64
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1-! with float
  TEST_INTERPRET("VARIABLE float-var");
  TEST_INTERPRET("4.5 float-var !");
  TEST_INTERPRET("float-var 1-!");
  TEST_INTERPRET("float-var @");
  TEST_STACK_TOP_FLOAT(3.5);
  TEST_INTERPRET("CELL-TYPE");
  TEST_STACK_TOP_INT(2);  // CELL_FLOAT
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test multiple 1-! operations
  TEST_INTERPRET("VARIABLE countdown");
  TEST_INTERPRET("5 countdown !");
  TEST_INTERPRET("countdown 1-!");
  TEST_INTERPRET("countdown 1-!");
  TEST_INTERPRET("countdown @");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1-! going negative
  TEST_INTERPRET("VARIABLE zero-var");
  TEST_INTERPRET("0 zero-var !");
  TEST_INTERPRET("zero-var 1-!");
  TEST_INTERPRET("zero-var @");
  TEST_STACK_TOP_INT(-1);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test 1-! (one minus store) error conditions
TEST_FUNCTION(test_one_minus_store_errors) {
  // Test 1-! with null pointer
  TEST_EXPECT_ERROR("NULL 1-!", "argument must be a pointer");

  // Test 1-! with non-pointer types
  TEST_EXPECT_ERROR("42 1-!", "argument must be a pointer");
  TEST_EXPECT_ERROR("\"hello\" 1-!", "argument must be a pointer");
  TEST_EXPECT_ERROR("3.14 1-!", "argument must be a pointer");
  TEST_EXPECT_ERROR("FALSE 1-!", "argument must be a pointer");

  // Test 1-! with null pointer stored in variable
  TEST_INTERPRET("VARIABLE null-ptr-var2");
  TEST_INTERPRET("NULL null-ptr-var2 !");
  TEST_EXPECT_ERROR("null-ptr-var2 @ 1-!", "argument must be a pointer");

  // Test 1-! with non-numeric target
  TEST_INTERPRET("VARIABLE string-target2");
  TEST_INTERPRET("\"world\" string-target2 !");
  TEST_EXPECT_ERROR("string-target2 1-!", "target must be numeric");

  TEST_INTERPRET("VARIABLE bool-target2");
  TEST_INTERPRET("TRUE bool-target2 !");
  TEST_EXPECT_ERROR("bool-target2 1-!", "target must be numeric");

  // Test 1-! stack underflow
  TEST_EXPECT_ERROR("1-!", "insufficient stack");
}

// Test complex memory operation combinations
TEST_FUNCTION(test_memory_operation_combinations) {
  // Test fetch-modify-store patterns
  TEST_INTERPRET("VARIABLE accumulator");
  TEST_INTERPRET("10 accumulator !");
  // Fetch, modify, store back
  TEST_INTERPRET("accumulator @ 5 + accumulator !");
  TEST_INTERPRET("accumulator @");
  TEST_STACK_TOP_INT(15);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test +! equivalent to fetch-add-store
  TEST_INTERPRET("VARIABLE compare-var1");
  TEST_INTERPRET("VARIABLE compare-var2");
  TEST_INTERPRET("100 compare-var1 !");
  TEST_INTERPRET("100 compare-var2 !");
  // Method 1: +!
  TEST_INTERPRET("25 compare-var1 +!");

  // Method 2: fetch-add-store
  TEST_INTERPRET("compare-var2 @ 25 + compare-var2 !");

  // Results should be identical
  TEST_INTERPRET("compare-var1 @ compare-var2 @ =");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test chained memory operations
  TEST_INTERPRET("VARIABLE chain-var");
  TEST_INTERPRET("1 chain-var !");
  TEST_INTERPRET("chain-var 1+!");   // Now 2
  TEST_INTERPRET("chain-var 1+!");   // Now 3
  TEST_INTERPRET("5 chain-var +!");  // Now 8
  TEST_INTERPRET("chain-var 1-!");   // Now 7
  TEST_INTERPRET("chain-var @");
  TEST_STACK_TOP_INT(7);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test edge cases and boundary conditions
TEST_FUNCTION(test_memory_operations_edge_cases) {
  // Test storing and fetching zero
  TEST_INTERPRET("VARIABLE zero-var");
  TEST_INTERPRET("0 zero-var !");
  TEST_INTERPRET("zero-var @");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test storing and fetching very large numbers
  TEST_INTERPRET("VARIABLE big-var");
  TEST_INTERPRET("2147483647 big-var !");  // INT32_MAX
  TEST_INTERPRET("big-var @");
  TEST_STACK_TOP_INT(2147483647);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test 1+! on INT32_MAX (potential overflow)
  TEST_INTERPRET("VARIABLE overflow-var");
  TEST_INTERPRET("2147483647 overflow-var !");
  TEST_INTERPRET("overflow-var 1+!");
  TEST_INTERPRET("overflow-var @");
  // Behavior is implementation-defined, but shouldn't crash
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test operations on negative numbers
  TEST_INTERPRET("VARIABLE neg-var");
  TEST_INTERPRET("-100 neg-var !");
  TEST_INTERPRET("50 neg-var +!");
  TEST_INTERPRET("neg-var @");
  TEST_STACK_TOP_INT(-50);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test float precision operations
  TEST_INTERPRET("VARIABLE precision-var");
  TEST_INTERPRET("0.1 precision-var !");
  TEST_INTERPRET("0.2 precision-var +!");
  TEST_INTERPRET("precision-var @");
  TEST_STACK_TOP_FLOAT(0.3);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all memory operations tests
void register_memory_ops_tests(void) {
  REGISTER_TEST(test_fetch_valid_pointers);
  REGISTER_TEST(test_fetch_errors);
  REGISTER_TEST(test_fetch_undefined);
  REGISTER_TEST(test_store_valid_operations);
  REGISTER_TEST(test_store_errors);
  REGISTER_TEST(test_plus_store_type_promotion);
  REGISTER_TEST(test_plus_store_errors);
  REGISTER_TEST(test_one_plus_store);
  REGISTER_TEST(test_one_plus_store_errors);
  REGISTER_TEST(test_one_minus_store);
  REGISTER_TEST(test_one_minus_store_errors);
  REGISTER_TEST(test_memory_operation_combinations);
  REGISTER_TEST(test_memory_operations_edge_cases);
}

#endif  // TEST_ENABLED