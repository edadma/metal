#include "test_array.h"

#ifdef TEST_ENABLED
#include <stdio.h>

#include "test.h"

// === BASIC INDEX@ FUNCTIONALITY TESTS ===

// Test INDEX@ with valid indices on simple arrays
TEST_FUNCTION(test_index_fetch_basic) {
  // Test fetching from array with integers
  TEST_INTERPRET("[] 10 , 20 , 30 ,");

  // Fetch element 0
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(10);
  TEST_INTERPRET("DROP");

  // Fetch element 1
  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_INT(20);
  TEST_INTERPRET("DROP");

  // Fetch element 2
  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_INT(30);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX@ with string array
TEST_FUNCTION(test_index_fetch_strings) {
  // Create array with strings
  TEST_INTERPRET("[] \"first\" , \"second\" , \"third\" ,");

  // Fetch string elements
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_STRING("first");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_STRING("second");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_STRING("third");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX@ with mixed type array
TEST_FUNCTION(test_index_fetch_mixed_types) {
  // Create array with mixed types
  TEST_INTERPRET("[] 42 , \"hello\" , 3.14 , TRUE ,");

  // Fetch each type
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 3 INDEX@");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// === BASIC INDEX! FUNCTIONALITY TESTS ===

// Test INDEX! storing different types
TEST_FUNCTION(test_index_store_basic) {
  // Create array with initial values
  TEST_INTERPRET("[] 0 , 0 , 0 ,");

  // Store different values at each position
  TEST_INTERPRET("100 OVER 0 INDEX!");
  TEST_INTERPRET("200 OVER 1 INDEX!");
  TEST_INTERPRET("300 OVER 2 INDEX!");

  // Verify the stored values
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(100);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_INT(200);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_INT(300);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX! with string storage
TEST_FUNCTION(test_index_store_strings) {
  // Create array with placeholder strings
  TEST_INTERPRET("[] \"\" , \"\" , \"\" ,");

  // Store strings at each position
  TEST_INTERPRET("\"alpha\" OVER 0 INDEX!");
  TEST_INTERPRET("\"beta\" OVER 1 INDEX!");
  TEST_INTERPRET("\"gamma\" OVER 2 INDEX!");

  // Verify stored strings
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_STRING("alpha");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_STRING("beta");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_STRING("gamma");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX! overwriting existing values
TEST_FUNCTION(test_index_store_overwrite) {
  // Create array with initial values
  TEST_INTERPRET("[] \"old1\" , \"old2\" , \"old3\" ,");

  // Overwrite middle element
  TEST_INTERPRET("\"new2\" OVER 1 INDEX!");

  // Verify overwrite worked and other elements unchanged
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_STRING("old1");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_STRING("new2");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_STRING("old3");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// === ERROR CASE TESTS ===

// Test INDEX@ error conditions
TEST_FUNCTION(test_index_fetch_errors) {
  // Test INDEX@ with out-of-bounds positive index
  TEST_INTERPRET("[] 1 , 2 ,");
  TEST_EXPECT_ERROR("DUP 5 INDEX@", "index out of bounds");

  // Test INDEX@ with negative index
  TEST_INTERPRET("[] 1 , 2 ,");
  TEST_EXPECT_ERROR("DUP -1 INDEX@", "index out of bounds");

  // Test INDEX@ with empty array
  TEST_INTERPRET("[]");
  TEST_EXPECT_ERROR("DUP 0 INDEX@", "index out of bounds");

  // Test INDEX@ with non-array types
  TEST_INTERPRET("[]");
  TEST_EXPECT_ERROR("42 0 INDEX@", "not an array");
  TEST_INTERPRET("[]");
  TEST_EXPECT_ERROR("\"hello\" 0 INDEX@", "not an array");
  TEST_INTERPRET("[]");
  TEST_EXPECT_ERROR("3.14 0 INDEX@", "not an array");
  TEST_INTERPRET("[]");
  TEST_EXPECT_ERROR("TRUE 0 INDEX@", "not an array");

  // Test INDEX@ with non-integer index
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("DUP \"bad\" INDEX@", "index must be integer");
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("DUP 1.5 INDEX@", "index must be integer");
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("DUP TRUE INDEX@", "index must be integer");

  // Test INDEX@ stack underflow
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("INDEX@", "insufficient stack");

  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("INDEX@", "insufficient stack");
}

// Test INDEX! error conditions
TEST_FUNCTION(test_index_store_errors) {
  // Test INDEX! with out-of-bounds positive index
  TEST_INTERPRET("[] 1 , 2 ,");
  TEST_EXPECT_ERROR("99 OVER 5 INDEX!", "index out of bounds");

  // Test INDEX! with negative index
  TEST_INTERPRET("[] 1 , 2 ,");
  TEST_EXPECT_ERROR("99 OVER -1 INDEX!", "index out of bounds");

  // Test INDEX! with empty array
  TEST_INTERPRET("[]");
  TEST_EXPECT_ERROR("99 OVER 0 INDEX!", "index out of bounds");

  // Test INDEX! with non-array types
  TEST_EXPECT_ERROR("99 42 0 INDEX!", "not an array");
  TEST_EXPECT_ERROR("99 \"hello\" 0 INDEX!", "not an array");
  TEST_EXPECT_ERROR("99 3.14 0 INDEX!", "not an array");
  TEST_EXPECT_ERROR("99 TRUE 0 INDEX!", "not an array");

  // Test INDEX! with non-integer index
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("99 OVER \"bad\" INDEX!", "index must be integer");
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("99 OVER 1.5 INDEX!", "index must be integer");
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("99 OVER TRUE INDEX!", "index must be integer");

  // Test INDEX! stack underflow
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("INDEX!", "insufficient stack");

  TEST_INTERPRET("99");
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("INDEX!", "insufficient stack");

  TEST_INTERPRET("99 [] 1 ,");
  TEST_INTERPRET("[] 1 ,");
  TEST_EXPECT_ERROR("INDEX!", "INDEX!: index must be integer");
}

// === EDGE CASE TESTS ===

// Test single element arrays
TEST_FUNCTION(test_single_element_array) {
  // Test INDEX@ with single element
  TEST_INTERPRET("[] 42 ,");
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  // Test INDEX! with single element
  TEST_INTERPRET("99 OVER 0 INDEX!");
  TEST_INTERPRET("0 INDEX@");
  TEST_STACK_TOP_INT(99);
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test boundary indices
TEST_FUNCTION(test_boundary_indices) {
  // Create array with known size
  TEST_INTERPRET("[] 10 , 20 , 30 , 40 , 50 ,");  // 5 elements

  // Test first index (0)
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(10);
  TEST_INTERPRET("DROP");

  // Test last valid index (4)
  TEST_INTERPRET("DUP 4 INDEX@");
  TEST_STACK_TOP_INT(50);
  TEST_INTERPRET("DROP");

  // Test just beyond last index (should error)
  TEST_EXPECT_ERROR("DUP 5 INDEX@", "index out of bounds");
}

// Test large arrays
TEST_FUNCTION(test_large_array_indexing) {
  // Create larger array (20 elements)
  TEST_INTERPRET("DEF T [] 0 , 1 BEGIN DUP 20 < WHILE OVER OVER , DROP 1+ REPEAT DROP END T");

  // Test accessing various positions
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 10 INDEX@");
  TEST_STACK_TOP_INT(10);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 19 INDEX@");
  TEST_STACK_TOP_INT(19);
  TEST_INTERPRET("DROP");

  // Test storing at various positions
  TEST_INTERPRET("999 OVER 5 INDEX!");
  TEST_INTERPRET("DUP 5 INDEX@");
  TEST_STACK_TOP_INT(999);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// === TYPE MIXING TESTS ===

// Test storing and fetching different types in same array
TEST_FUNCTION(test_type_mixing) {
  // Create array and store different types
  TEST_INTERPRET("[] NULL , NULL , NULL , NULL ,");  // 4 null placeholders

  // Store different types
  TEST_INTERPRET("42 OVER 0 INDEX!");        // int32
  TEST_INTERPRET("\"test\" OVER 1 INDEX!");  // string
  TEST_INTERPRET("3.14 OVER 2 INDEX!");      // float
  TEST_INTERPRET("TRUE OVER 3 INDEX!");      // boolean

  // Fetch and verify each type
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_STRING("test");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 3 INDEX@");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test storing arrays within arrays
TEST_FUNCTION(test_nested_array_storage) {
  // Create parent array
  TEST_INTERPRET("[] NULL , NULL ,");  // 2 slots

  // Create sub-arrays
  TEST_INTERPRET("[] 1 , 2 , OVER 0 INDEX!");          // Store sub-array at position 0
  TEST_INTERPRET("[] \"a\" , \"b\" , OVER 1 INDEX!");  // Store sub-array at position 1

  // Fetch and verify nested arrays
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_INTERPRET("DUP 0 INDEX@");  // Should be 1
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("1 INDEX@");  // Should be 2
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_INTERPRET("DUP 0 INDEX@");  // Should be "a"
  TEST_STACK_TOP_STRING("a");
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("1 INDEX@");  // Should be "b"
  TEST_STACK_TOP_STRING("b");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// === REFERENCE COUNTING INTEGRATION TESTS ===

// Test INDEX@ doesn't affect refcounts incorrectly
TEST_FUNCTION(test_index_fetch_refcount_safety) {
  // Create array with allocated string
  TEST_INTERPRET("[] \"refcount-test\" ,");

  // Check initial refcount (array holds reference)
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Array reference + fetched copy
  TEST_INTERPRET("DROP");

  // Multiple fetches shouldn't change original refcount permanently
  TEST_INTERPRET("DUP 0 INDEX@ DROP");  // Fetch and immediately drop
  TEST_INTERPRET("DUP 0 INDEX@ DROP");  // Fetch and immediately drop
  TEST_INTERPRET("DUP 0 INDEX@");       // Fetch again
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Should still be 2
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX! properly handles reference counting
TEST_FUNCTION(test_index_store_refcount_management) {
  // Take memory snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create array with string
  TEST_INTERPRET("[] \"original\" ,");

  // Store new string, overwriting old one (old should be released)
  TEST_INTERPRET("\"replacement\" OVER 0 INDEX!");

  // Verify new string is stored
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_STRING("replacement");
  TEST_INTERPRET("DROP");

  // Clean up array
  TEST_INTERPRET("DROP");

  // Check no memory leaks (original string should have been released)
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test INDEX! with shared references
TEST_FUNCTION(test_index_store_shared_references) {
  // Create shared string
  TEST_INTERPRET("\"shared\"");

  // Create two arrays and store shared string in both
  TEST_INTERPRET("DUP [] SWAP ,");  // Array1 with shared string
  TEST_INTERPRET("[] ROT ,");       // Array2 with shared string

  // Both arrays should reference the same string
  TEST_INTERPRET("OVER 0 INDEX@");  // Get string from array1
  TEST_INTERPRET("OVER 0 INDEX@");  // Get string from array2
  TEST_INTERPRET("CELL-SHARED?");   // Should be the same object
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Check refcount of shared string (should be 3: array1 + array2 + 1 fetched copies)
  TEST_INTERPRET("OVER 0 INDEX@");
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// === STACK EFFECT TESTS ===

// Test INDEX@ stack signature exactly
TEST_FUNCTION(test_index_fetch_stack_effects) {
  // Create test array
  TEST_INTERPRET("[] 42 , 99 ,");
  TEST_INTERPRET("\"marker\"");  // Add marker to verify stack position

  // Stack should be: [array, "marker"]
  TEST_STACK_DEPTH(2);

  // INDEX@ should consume array and index, produce value
  TEST_INTERPRET("SWAP 1 INDEX@");  // Get element 1 from array

  // Stack should now be: ["marker", 99]
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(99);
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_STRING("marker");
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test INDEX! stack signature exactly
TEST_FUNCTION(test_index_store_stack_effects) {
  // Create test array and setup stack
  TEST_INTERPRET("[] 0 , 0 ,");
  TEST_INTERPRET("\"marker\"");
  TEST_INTERPRET("77");

  // Stack should be: [array, "marker", 77]
  TEST_STACK_DEPTH(3);

  // INDEX! should consume value, array, and index
  TEST_INTERPRET("ROT 1 INDEX!");  // Store 77 at position 1 in array

  // Stack should now be: ["marker"]
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_STRING("marker");
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// === COMPLEX SCENARIOS ===

// Test round-trip operations
TEST_FUNCTION(test_round_trip_operations) {
  // Create array
  TEST_INTERPRET("[] 10 , 20 , 30 ,");

  // Fetch, modify, store back
  TEST_INTERPRET("DUP 1 INDEX@");   // Fetch element 1 (20)
  TEST_INTERPRET("100 +");          // Add 100 (now 120)
  TEST_INTERPRET("OVER 1 INDEX!");  // Store back at position 1

  // Verify the modification
  TEST_INTERPRET("DUP 1 INDEX@");
  TEST_STACK_TOP_INT(120);
  TEST_INTERPRET("DROP");

  // Verify other elements unchanged
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_INT(10);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_INT(30);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test array resizing scenarios (if supported)
TEST_FUNCTION(test_dynamic_array_access) {
  // Create small array
  TEST_INTERPRET("[] 1 , 2 ,");

  // Add more elements
  TEST_INTERPRET("3 , 4 , 5 ,");

  // Test accessing newly added elements
  TEST_INTERPRET("DUP 2 INDEX@");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 3 INDEX@");
  TEST_STACK_TOP_INT(4);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 4 INDEX@");
  TEST_STACK_TOP_INT(5);
  TEST_INTERPRET("DROP");

  // Modify elements in expanded array
  TEST_INTERPRET("999 OVER 3 INDEX!");
  TEST_INTERPRET("4 INDEX@");
  TEST_STACK_TOP_INT(5);  // Element 4 should be unchanged
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test memory leak scenarios with indexing
TEST_FUNCTION(test_index_memory_leak_detection) {
  // Take snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create array with allocated strings
  TEST_INTERPRET("[] \"str1\" , \"str2\" , \"str3\" ,");

  // Multiple fetch operations (should not leak)
  TEST_INTERPRET("DUP 0 INDEX@ DROP");
  TEST_INTERPRET("DUP 1 INDEX@ DROP");
  TEST_INTERPRET("DUP 2 INDEX@ DROP");

  // Multiple store operations (should not leak old values)
  TEST_INTERPRET("\"new1\" OVER 0 INDEX!");
  TEST_INTERPRET("\"new2\" OVER 1 INDEX!");
  TEST_INTERPRET("\"new3\" OVER 2 INDEX!");

  // More fetch operations on new values
  TEST_INTERPRET("DUP 0 INDEX@ DROP");
  TEST_INTERPRET("DUP 1 INDEX@ DROP");
  TEST_INTERPRET("DUP 2 INDEX@ DROP");

  // Clean up array
  TEST_INTERPRET("DROP");

  // Check for leaks
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// === INTEGRATION WITH EXISTING REFERENCE COUNTING TESTS ===

// Test using INDEX@ in cascading reference scenarios
TEST_FUNCTION(test_index_cascading_references) {
  // Take snapshot for leak detection
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create nested structure: parent array containing two sub-arrays
  TEST_INTERPRET("[] [] \"a\" , \"b\" , , [] \"c\" , \"d\" , ,");

  // Verify structure using INDEX@
  TEST_INTERPRET("DUP 0 INDEX@");  // Get first sub-array
  TEST_INTERPRET("DUP 0 INDEX@");  // Get "a" from first sub-array
  TEST_STACK_TOP_STRING("a");
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("1 INDEX@");  // Get "b" from first sub-array
  TEST_STACK_TOP_STRING("b");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP 1 INDEX@");  // Get second sub-array
  TEST_INTERPRET("DUP 0 INDEX@");  // Get "c" from second sub-array
  TEST_STACK_TOP_STRING("c");
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("1 INDEX@");  // Get "d" from second sub-array
  TEST_STACK_TOP_STRING("d");
  TEST_INTERPRET("DROP");

  // Drop the parent array - should cascade release all nested elements
  TEST_INTERPRET("DROP");

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test INDEX! replacing nested structures
TEST_FUNCTION(test_index_replace_nested_structures) {
  // Take snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create parent array with nested sub-array
  TEST_INTERPRET("[] [] \"old-a\" , \"old-b\" , ,");

  // Replace the nested array with a new one
  TEST_INTERPRET("[] \"new-x\" , \"new-y\" , OVER 0 INDEX!");

  // Verify replacement worked
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_INTERPRET("DUP 0 INDEX@");
  TEST_STACK_TOP_STRING("new-x");
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("1 INDEX@");
  TEST_STACK_TOP_STRING("new-y");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");

  // Check no leaks (old nested array should have been released)
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// === REGISTRATION ===

void register_array_tests(void) {
  // Basic functionality tests
  REGISTER_TEST(test_index_fetch_basic);
  REGISTER_TEST(test_index_fetch_strings);
  REGISTER_TEST(test_index_fetch_mixed_types);
  REGISTER_TEST(test_index_store_basic);
  REGISTER_TEST(test_index_store_strings);
  REGISTER_TEST(test_index_store_overwrite);

  // Error case tests
  REGISTER_TEST(test_index_fetch_errors);
  REGISTER_TEST(test_index_store_errors);

  // Edge case tests
  REGISTER_TEST(test_single_element_array);
  REGISTER_TEST(test_boundary_indices);
  REGISTER_TEST(test_large_array_indexing);

  // Type mixing tests
  REGISTER_TEST(test_type_mixing);
  REGISTER_TEST(test_nested_array_storage);

  // Reference counting integration
  REGISTER_TEST(test_index_fetch_refcount_safety);
  REGISTER_TEST(test_index_store_refcount_management);
  REGISTER_TEST(test_index_store_shared_references);

  // Stack effect tests
  REGISTER_TEST(test_index_fetch_stack_effects);
  REGISTER_TEST(test_index_store_stack_effects);

  // Complex scenarios
  REGISTER_TEST(test_round_trip_operations);
  REGISTER_TEST(test_dynamic_array_access);
  REGISTER_TEST(test_index_memory_leak_detection);
  REGISTER_TEST(test_index_cascading_references);
  REGISTER_TEST(test_index_replace_nested_structures);
}

#endif  // TEST_ENABLED