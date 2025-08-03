#include "test_object.h"

#ifdef TEST_ENABLED
#include "test.h"

// === BASIC OBJECT INDEX@ FUNCTIONALITY TESTS ===

// Test INDEX@ with valid object properties
TEST_FUNCTION(test_object_index_fetch_basic) {
  // Create object with properties using INDEX!
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"John\" OVER \"name\" INDEX!");
  TEST_INTERPRET("25 OVER \"age\" INDEX!");
  TEST_INTERPRET("TRUE OVER \"active\" INDEX!");

  // Fetch each property
  TEST_INTERPRET("DUP \"name\" INDEX@");
  TEST_STACK_TOP_STRING("John");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"age\" INDEX@");
  TEST_STACK_TOP_INT(25);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"active\" INDEX@");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX@ with missing properties returns undefined
TEST_FUNCTION(test_object_index_fetch_missing) {
  // Create object with one property
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"value\" OVER \"key\" INDEX!");

  // Fetch existing property
  TEST_INTERPRET("DUP \"key\" INDEX@");
  TEST_STACK_TOP_STRING("value");
  TEST_INTERPRET("DROP");

  // Fetch missing property
  TEST_INTERPRET("DUP \"missing\" INDEX@");
  TEST_INTERPRET("UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX@ with empty object
TEST_FUNCTION(test_object_index_fetch_empty) {
  // Create empty object
  TEST_INTERPRET("{}");

  // Fetch from empty object should return undefined
  TEST_INTERPRET("DUP \"anything\" INDEX@");
  TEST_INTERPRET("UNDEFINED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// === BASIC OBJECT INDEX! FUNCTIONALITY TESTS ===

// Test INDEX! storing different types
TEST_FUNCTION(test_object_index_store_basic) {
  // Create empty object
  TEST_INTERPRET("{}");

  // Store different types
  TEST_INTERPRET("42 OVER \"number\" INDEX!");
  TEST_INTERPRET("\"hello\" OVER \"text\" INDEX!");
  TEST_INTERPRET("3.14 OVER \"pi\" INDEX!");
  TEST_INTERPRET("FALSE OVER \"flag\" INDEX!");

  // Verify stored values
  TEST_INTERPRET("DUP \"number\" INDEX@");
  TEST_STACK_TOP_INT(42);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"text\" INDEX@");
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"pi\" INDEX@");
  TEST_STACK_TOP_FLOAT(3.14);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"flag\" INDEX@");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test INDEX! overwriting existing properties
TEST_FUNCTION(test_object_index_store_overwrite) {
  // Create object with initial values
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"old-value\" OVER \"key1\" INDEX!");
  TEST_INTERPRET("100 OVER \"key2\" INDEX!");

  // Overwrite properties
  TEST_INTERPRET("\"new-value\" OVER \"key1\" INDEX!");
  TEST_INTERPRET("200 OVER \"key2\" INDEX!");

  // Verify overwrites worked
  TEST_INTERPRET("DUP \"key1\" INDEX@");
  TEST_STACK_TOP_STRING("new-value");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"key2\" INDEX@");
  TEST_STACK_TOP_INT(200);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// === ERROR CASE TESTS ===

// Test INDEX@ error conditions with objects
TEST_FUNCTION(test_object_index_fetch_errors) {
  // Test INDEX@ with non-string key on object
  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("DUP 42 INDEX@", "object index must be string");

  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("DUP 3.14 INDEX@", "object index must be string");

  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("DUP TRUE INDEX@", "object index must be string");

  // Test INDEX@ with non-object, non-array types (should still work for arrays)
  TEST_EXPECT_ERROR("42 \"key\" INDEX@", "container must be array or object");
  TEST_EXPECT_ERROR("\"hello\" \"key\" INDEX@", "container must be array or object");
  TEST_EXPECT_ERROR("3.14 \"key\" INDEX@", "container must be array or object");
  TEST_EXPECT_ERROR("TRUE \"key\" INDEX@", "container must be array or object");

  // Test INDEX@ stack underflow
  TEST_EXPECT_ERROR("INDEX@", "insufficient stack");
  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("INDEX@", "insufficient stack");
}

// Test INDEX! error conditions with objects
TEST_FUNCTION(test_object_index_store_errors) {
  // Test INDEX! with non-string key on object
  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("\"value\" OVER 42 INDEX!", "object index must be string");

  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("\"value\" OVER 3.14 INDEX!", "object index must be string");

  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("\"value\" OVER TRUE INDEX!", "object index must be string");

  // Test INDEX! with non-object, non-array types
  TEST_EXPECT_ERROR("\"value\" 42 \"key\" INDEX!", "container must be array or object");
  TEST_EXPECT_ERROR("\"value\" \"hello\" \"key\" INDEX!", "container must be array or object");
  TEST_EXPECT_ERROR("\"value\" 3.14 \"key\" INDEX!", "container must be array or object");
  TEST_EXPECT_ERROR("\"value\" TRUE \"key\" INDEX!", "container must be array or object");

  // Test INDEX! stack underflow
  TEST_EXPECT_ERROR("INDEX!", "insufficient stack");

  TEST_INTERPRET("\"value\"");
  TEST_EXPECT_ERROR("INDEX!", "insufficient stack");

  TEST_INTERPRET("\"value\" {}");
  TEST_EXPECT_ERROR("INDEX!", "insufficient stack");
}

// === OBJECT-SPECIFIC WORD TESTS ===

// Test PUT word (object builder)
TEST_FUNCTION(test_put_word) {
  // Test basic PUT functionality
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"name\" \"Alice\" PUT");
  TEST_INTERPRET("\"age\" 30 PUT");

  // Verify PUT worked
  TEST_INTERPRET("DUP \"name\" INDEX@");
  TEST_STACK_TOP_STRING("Alice");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"age\" INDEX@");
  TEST_STACK_TOP_INT(30);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test PUT error conditions
TEST_FUNCTION(test_put_errors) {
  // Test PUT with non-object
  TEST_EXPECT_ERROR("42 \"key\" \"value\" PUT", "first argument must be object");
  TEST_EXPECT_ERROR("[] \"key\" \"value\" PUT", "first argument must be object");
  TEST_EXPECT_ERROR("\"string\" \"key\" \"value\" PUT", "first argument must be object");

  // Test PUT with non-string key
  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("42 \"value\" PUT", "key must be string");

  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("3.14 \"value\" PUT", "key must be string");

  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("TRUE \"value\" PUT", "key must be string");

  // Test PUT stack underflow
  TEST_EXPECT_ERROR("PUT", "insufficient stack");

  TEST_INTERPRET("{}");
  TEST_EXPECT_ERROR("PUT", "insufficient stack");

  TEST_INTERPRET("{} \"key\"");
  TEST_EXPECT_ERROR("PUT", "insufficient stack");
}

// Test HAS? word
TEST_FUNCTION(test_has_word) {
  // Create object with properties
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"value1\" OVER \"key1\" INDEX!");
  TEST_INTERPRET("\"value2\" OVER \"key2\" INDEX!");

  // Test HAS? with existing keys
  TEST_INTERPRET("DUP \"key1\" HAS?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"key2\" HAS?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Test HAS? with missing key
  TEST_INTERPRET("DUP \"missing\" HAS?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Test HAS? with empty object
  TEST_INTERPRET("DROP {}");
  TEST_INTERPRET("DUP \"anything\" HAS?");
  TEST_STACK_TOP_BOOLEAN(false);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test KEYS word
TEST_FUNCTION(test_keys_word) {
  // Test KEYS with empty object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("DUP KEYS");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  // Test KEYS with populated object
  TEST_INTERPRET("\"alpha\" OVER \"a\" INDEX!");
  TEST_INTERPRET("\"beta\" OVER \"b\" INDEX!");
  TEST_INTERPRET("\"gamma\" OVER \"c\" INDEX!");

  TEST_INTERPRET("DUP KEYS");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test VALUES word
TEST_FUNCTION(test_values_word) {
  // Test VALUES with empty object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("DUP VALUES");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  // Test VALUES with populated object
  TEST_INTERPRET("10 OVER \"x\" INDEX!");
  TEST_INTERPRET("20 OVER \"y\" INDEX!");
  TEST_INTERPRET("30 OVER \"z\" INDEX!");

  TEST_INTERPRET("DUP VALUES");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test ENTRIES word
TEST_FUNCTION(test_entries_word) {
  // Test ENTRIES with empty object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("DUP ENTRIES");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");

  // Test ENTRIES with populated object
  TEST_INTERPRET("\"John\" OVER \"name\" INDEX!");
  TEST_INTERPRET("25 OVER \"age\" INDEX!");

  TEST_INTERPRET("DUP ENTRIES");
  TEST_INTERPRET("DUP LENGTH");
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");

  // Check that entries are [key value] pairs
  TEST_INTERPRET("DUP 0 INDEX@");  // First entry
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(2);  // Should be 2-element array
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Test FROM-ENTRIES word
TEST_FUNCTION(test_from_entries_word) {
  // Test FROM-ENTRIES with empty array
  TEST_INTERPRET("[]");
  TEST_INTERPRET("FROM-ENTRIES");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test FROM-ENTRIES with valid entries
  TEST_INTERPRET("[]");
  TEST_INTERPRET("[] \"name\" , \"Alice\" , ,");  // ["name", "Alice"] pair
  TEST_INTERPRET("[] \"age\" , 30 , ,");          // ["age", 30] pair
  TEST_INTERPRET("FROM-ENTRIES");

  // Verify the resulting object
  TEST_INTERPRET("DUP \"name\" INDEX@");
  TEST_STACK_TOP_STRING("Alice");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"age\" INDEX@");
  TEST_STACK_TOP_INT(30);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test FROM-ENTRIES error conditions
TEST_FUNCTION(test_from_entries_errors) {
  // Test FROM-ENTRIES with non-array
  TEST_EXPECT_ERROR("42 FROM-ENTRIES", "argument must be array");
  TEST_EXPECT_ERROR("{} FROM-ENTRIES", "argument must be array");
  TEST_EXPECT_ERROR("\"string\" FROM-ENTRIES", "argument must be array");

  // Test FROM-ENTRIES with invalid entry format
  TEST_INTERPRET("[] 42 ,");  // Non-array entry
  TEST_EXPECT_ERROR("FROM-ENTRIES", "each entry must be array");

  // Test FROM-ENTRIES with wrong entry length
  TEST_INTERPRET("[] [] \"key\" , ,");  // Single element array
  TEST_EXPECT_ERROR("FROM-ENTRIES", "each entry must be [key value] pair");

  TEST_INTERPRET("[] [] \"key\" , \"val\" , \"extra\" , ,");  // Three elements
  TEST_EXPECT_ERROR("FROM-ENTRIES", "each entry must be [key value] pair");

  // Test FROM-ENTRIES with non-string key
  TEST_INTERPRET("[] [] 42 , \"value\" , ,");
  TEST_EXPECT_ERROR("FROM-ENTRIES", "entry key must be string");
}

// === OBJECT-ARRAY INTEGRATION TESTS ===

// Test round-trip object to array to object
TEST_FUNCTION(test_object_array_round_trip) {
  // Create object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"Alice\" OVER \"name\" INDEX!");
  TEST_INTERPRET("30 OVER \"age\" INDEX!");
  TEST_INTERPRET("TRUE OVER \"active\" INDEX!");

  // Convert to entries and back
  TEST_INTERPRET("DUP ENTRIES");
  TEST_INTERPRET("FROM-ENTRIES");

  // Verify round-trip worked
  TEST_INTERPRET("DUP \"name\" INDEX@");
  TEST_STACK_TOP_STRING("Alice");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"age\" INDEX@");
  TEST_STACK_TOP_INT(30);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"active\" INDEX@");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// === REFERENCE COUNTING TESTS ===

// Test object INDEX@ doesn't affect refcounts incorrectly
TEST_FUNCTION(test_object_index_fetch_refcount_safety) {
  // Create object with allocated string property
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"refcount-test\" OVER \"key\" INDEX!");

  // Check initial refcount (object holds reference)
  TEST_INTERPRET("DUP \"key\" INDEX@");
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Object reference + fetched copy
  TEST_INTERPRET("DROP");

  // Multiple fetches shouldn't change original refcount permanently
  TEST_INTERPRET("DUP \"key\" INDEX@ DROP");  // Fetch and immediately drop
  TEST_INTERPRET("DUP \"key\" INDEX@ DROP");  // Fetch and immediately drop
  TEST_INTERPRET("DUP \"key\" INDEX@");       // Fetch again
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Should still be 2
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test object INDEX! properly manages refcounts when storing/replacing
TEST_FUNCTION(test_object_index_store_refcount_management) {
  // Create object and initial string
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"initial-value\"");
  // Check initial string refcount
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Only our reference
  TEST_INTERPRET("DROP");

  // Store in object (should increment refcount)
  TEST_INTERPRET("DUP 2 PICK \"key\" INDEX!");
  // Check refcount increased
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(3);  // Our reference + object's reference
  TEST_INTERPRET("DROP");

  // Replace with new value (should decrement old, increment new)
  TEST_INTERPRET("\"replacement-value\"");
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Only our reference
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("DUP 3 PICK \"key\" INDEX!");  // Store replacement
                                                // Check new value refcount
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Our reference + object's reference
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Test shared object references
TEST_FUNCTION(test_object_shared_references) {
  // Create an object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"shared-value\" OVER \"key\" INDEX!");
  // Check object refcount
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Only our reference
  TEST_INTERPRET("DROP");

  // Duplicate object reference
  TEST_INTERPRET("DUP");
  // Check refcount increased
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(3);  // Two references now
  TEST_INTERPRET("DROP");

  // Modify through one reference
  TEST_INTERPRET("\"new-value\" OVER \"key\" INDEX!");
  // Check modification visible through other reference
  TEST_INTERPRET("DUP \"key\" INDEX@");
  TEST_STACK_TOP_STRING("new-value");
  TEST_INTERPRET("DROP");

  // Drop one reference
  TEST_INTERPRET("DROP");
  // Check refcount decreased
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(1);  // Back to one reference
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_STACK_DEPTH(0);
}

// Test refcount cascading with nested objects
TEST_FUNCTION(test_object_nested_refcount_cascading) {
  // Take memory snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create nested object structure
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"inner-value\" OVER \"inner-key\" INDEX!");  // Inner object
  TEST_INTERPRET("DUP");                                        // Duplicate inner object
  TEST_INTERPRET("{}");                                         // Create outer object
  TEST_INTERPRET("ROT OVER \"nested\" INDEX!");                 // Store inner in outer
                                                                // We should have: [inner-obj outer-obj]
  // Inner object should have refcount 2 (our copy + outer's reference)
  TEST_INTERPRET("OVER REFCOUNT");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");

  // Drop our direct reference to inner object
  TEST_INTERPRET("SWAP DROP");
  // Access inner object through outer object
  TEST_INTERPRET("DUP \"nested\" INDEX@");
  TEST_INTERPRET("\"inner-key\" INDEX@");
  TEST_STACK_TOP_STRING("inner-value");
  TEST_INTERPRET("DROP");

  // Clean up outer object (should cascade release inner object)
  TEST_INTERPRET("DROP");

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test object property replacement releases old values
TEST_FUNCTION(test_object_property_replacement_release) {
  // Take memory snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create object with initial array property
  TEST_INTERPRET("{}");
  TEST_INTERPRET("[] \"old-item\" , OVER \"array-prop\" INDEX!");

  // Replace with new array (old array should be released)
  TEST_INTERPRET("[] \"new-item\" , OVER \"array-prop\" INDEX!");

  // Replace with string (new array should be released)
  TEST_INTERPRET("\"string-value\" OVER \"array-prop\" INDEX!");

  // Replace with different string (old string should be released)
  TEST_INTERPRET("\"final-string\" OVER \"array-prop\" INDEX!");

  // Clean up object
  TEST_INTERPRET("DROP");

  // Check no leaks occurred (all intermediate values should be released)
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test object operations with REFCOUNT-EXPECT utility
TEST_FUNCTION(test_object_refcount_expect_utility) {
  // Create object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"test-value\" OVER \"key\" INDEX!");
  // Test REFCOUNT-EXPECT with object
  TEST_INTERPRET("DUP 2 REFCOUNT-EXPECT");  // Should have refcount 1
                                            // Duplicate object
  TEST_INTERPRET("DUP");
  TEST_INTERPRET("DUP 3 REFCOUNT-EXPECT");  // Should have refcount 2
                                            // Access property (shouldn't change object refcount)
  TEST_INTERPRET("DUP \"key\" INDEX@");
  TEST_INTERPRET("DROP");
  TEST_INTERPRET("DUP 3 REFCOUNT-EXPECT");  // Should still be 2
                                            // Clean up
  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Test object creation and immediate release
TEST_FUNCTION(test_object_immediate_release) {
  // Take memory snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create and immediately drop empty object
  TEST_INTERPRET("{} DROP");

  // Create object with properties and immediately drop
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"value1\" OVER \"key1\" INDEX!");
  TEST_INTERPRET("\"value2\" OVER \"key2\" INDEX!");
  TEST_INTERPRET("[] \"item\" , OVER \"array\" INDEX!");
  TEST_INTERPRET("DROP");

  // Check no leaks
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test object array operations maintain refcounts
TEST_FUNCTION(test_object_array_operations_refcount) {
  // Take memory snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"test-value\" OVER \"key\" INDEX!");

  // Convert to entries (should maintain object refcount)
  TEST_INTERPRET("DUP ENTRIES");
  // Original object should still be valid
  TEST_INTERPRET("OVER \"key\" INDEX@");
  TEST_STACK_TOP_STRING("test-value");
  TEST_INTERPRET("DROP");

  // Convert back from entries
  TEST_INTERPRET("FROM-ENTRIES");
  // Should have same content
  TEST_INTERPRET("DUP \"key\" INDEX@");
  TEST_STACK_TOP_STRING("test-value");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP DROP");

  // Check no leaks
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// === MEMORY MANAGEMENT TESTS ===

// Test object memory leak detection
TEST_FUNCTION(test_object_memory_leak_detection) {
  // Take snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create object with allocated strings
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"string1\" OVER \"key1\" INDEX!");
  TEST_INTERPRET("\"string2\" OVER \"key2\" INDEX!");
  TEST_INTERPRET("\"string3\" OVER \"key3\" INDEX!");

  // Multiple property access operations (should not leak)
  TEST_INTERPRET("DUP \"key1\" INDEX@ DROP");
  TEST_INTERPRET("DUP \"key2\" INDEX@ DROP");
  TEST_INTERPRET("DUP \"key3\" INDEX@ DROP");

  // Property replacement operations (should release old values)
  TEST_INTERPRET("\"new1\" OVER \"key1\" INDEX!");
  TEST_INTERPRET("\"new2\" OVER \"key2\" INDEX!");
  TEST_INTERPRET("\"new3\" OVER \"key3\" INDEX!");

  // Clean up object
  TEST_INTERPRET("DROP");

  // Check for leaks
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test nested object structures
TEST_FUNCTION(test_nested_object_structures) {
  // Take snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create nested objects
  TEST_INTERPRET("{}");
  TEST_INTERPRET("{} \"inner\" \"value\" PUT OVER \"nested\" INDEX!");
  TEST_INTERPRET("[] \"item1\" , \"item2\" , OVER \"array\" INDEX!");

  // Access nested properties
  TEST_INTERPRET("DUP \"nested\" INDEX@");
  TEST_INTERPRET("\"inner\" INDEX@");
  TEST_STACK_TOP_STRING("value");
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP \"array\" INDEX@");
  TEST_INTERPRET("0 INDEX@");
  TEST_STACK_TOP_STRING("item1");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// === POLYMORPHIC LENGTH TESTS ===

// Test LENGTH with objects
TEST_FUNCTION(test_object_length) {
  // Test LENGTH with empty object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(0);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);

  // Test LENGTH with populated object
  TEST_INTERPRET("{}");
  TEST_INTERPRET("\"value1\" OVER \"key1\" INDEX!");
  TEST_INTERPRET("\"value2\" OVER \"key2\" INDEX!");
  TEST_INTERPRET("\"value3\" OVER \"key3\" INDEX!");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// === EDGE CASE TESTS ===

// Test objects with special key names
TEST_FUNCTION(test_object_special_keys) {
  TEST_INTERPRET("{}");

  // Test empty string key
  TEST_INTERPRET("\"value\" OVER \"\" INDEX!");
  TEST_INTERPRET("DUP \"\" INDEX@");
  TEST_STACK_TOP_STRING("value");
  TEST_INTERPRET("DROP");

  // Test keys with special characters
  TEST_INTERPRET("\"val1\" OVER \"key with spaces\" INDEX!");
  TEST_INTERPRET("\"val2\" OVER \"key-with-dashes\" INDEX!");
  TEST_INTERPRET("\"val3\" OVER \"key_with_underscores\" INDEX!");

  TEST_INTERPRET("DUP \"key with spaces\" INDEX@");
  TEST_STACK_TOP_STRING("val1");
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test large objects
TEST_FUNCTION(test_large_object) {
  // Add many properties (test object growth)
  TEST_INTERPRET("DEF add-props ");
  TEST_INTERPRET("  {} 0 BEGIN DUP 20 < WHILE");
  TEST_INTERPRET("    \"key-\" OVER +");      // Create key like "key0", "key1", etc
  TEST_INTERPRET("    2 PICK");               // Duplicate object, position key
  TEST_INTERPRET("    \"value-\" 3 PICK +");  // Create value like "value0", etc
  TEST_INTERPRET("    INDEX!");               // Store key-value pair
  TEST_INTERPRET("    1+");
  TEST_INTERPRET("  REPEAT DROP");
  TEST_INTERPRET("END");

  TEST_INTERPRET("add-props");
  TEST_INTERPRET("LENGTH");
  TEST_STACK_TOP_INT(20);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Register all object tests
void register_object_tests(void) {
  // Basic INDEX@ tests
  REGISTER_TEST(test_object_index_fetch_basic);
  REGISTER_TEST(test_object_index_fetch_missing);
  REGISTER_TEST(test_object_index_fetch_empty);

  // Basic INDEX! tests
  REGISTER_TEST(test_object_index_store_basic);
  REGISTER_TEST(test_object_index_store_overwrite);

  // Error case tests
  REGISTER_TEST(test_object_index_fetch_errors);
  REGISTER_TEST(test_object_index_store_errors);

  // Object-specific word tests
  REGISTER_TEST(test_put_word);
  REGISTER_TEST(test_put_errors);
  REGISTER_TEST(test_has_word);
  REGISTER_TEST(test_keys_word);
  REGISTER_TEST(test_values_word);
  REGISTER_TEST(test_entries_word);
  REGISTER_TEST(test_from_entries_word);
  REGISTER_TEST(test_from_entries_errors);

  // Reference counting tests
  REGISTER_TEST(test_object_index_fetch_refcount_safety);
  REGISTER_TEST(test_object_index_store_refcount_management);
  REGISTER_TEST(test_object_shared_references);
  REGISTER_TEST(test_object_nested_refcount_cascading);
  REGISTER_TEST(test_object_property_replacement_release);
  REGISTER_TEST(test_object_refcount_expect_utility);
  REGISTER_TEST(test_object_immediate_release);
  REGISTER_TEST(test_object_array_operations_refcount);

  // Integration tests
  REGISTER_TEST(test_object_array_round_trip);

  // Memory management tests
  REGISTER_TEST(test_object_memory_leak_detection);
  REGISTER_TEST(test_nested_object_structures);

  // Polymorphic tests
  REGISTER_TEST(test_object_length);

  // Edge case tests
  REGISTER_TEST(test_object_special_keys);
  REGISTER_TEST(test_large_object);
}

#endif  // TEST_ENABLED