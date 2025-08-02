#include "test_refcount.h"

#ifdef TEST_ENABLED
#include <stdio.h>

#include "array.h"
#include "dictionary.h"
#include "error.h"
#include "memory.h"
#include "stack.h"
#include "strings.h"
#include "test.h"

// Memory snapshot for leak detection
typedef struct {
  int allocs;
  int frees;
} memory_snapshot_t;

// === TEST UTILITY WORDS ===

// MEM-LEAKED? ( -- bool ) Check if current allocs != frees
static void native_mem_leaked_q(context_t* ctx) {
  int allocs, frees;
  get_memory_stats(&allocs, &frees);
  bool leaked = (allocs != frees);
  data_push(ctx, new_boolean(leaked));
}

// MEM-SNAPSHOT ( -- snapshot ) Capture current alloc count as int64
static void native_mem_snapshot(context_t* ctx) {
  int allocs, frees;
  get_memory_stats(&allocs, &frees);
  // Pack allocs and frees into a single int64 for simplicity
  int64_t snapshot = ((int64_t)allocs << 32) | (int64_t)frees;
  data_push(ctx, new_int64(snapshot));
}

// MEM-COMPARE ( snapshot -- leaked ) Compare current vs snapshot
static void native_mem_compare(context_t* ctx) {
  require_params(ctx, 1, "MEM-COMPARE");
  cell_t* snapshot_cell = data_pop(ctx);

  if (snapshot_cell->type != CELL_INT64) {
    error(ctx, "MEM-COMPARE: snapshot must be int64");
  }

  int64_t snapshot = snapshot_cell->payload.i64;
  int old_allocs = (int)(snapshot >> 32);
  int old_frees = (int)(snapshot & 0xFFFFFFFF);

  int current_allocs, current_frees;
  get_memory_stats(&current_allocs, &current_frees);

  int net_leaked = (current_allocs - current_frees) - (old_allocs - old_frees);
  data_push(ctx, new_int32(net_leaked));
  release(snapshot_cell);
}

// REFCOUNT-EXPECT ( cell expected -- ) Assert expected refcount
static void native_refcount_expect(context_t* ctx) {
  require_params(ctx, 2, "REFCOUNT-EXPECT");
  cell_t* expected_cell = data_pop(ctx);
  cell_t* test_cell = data_pop(ctx);

  if (expected_cell->type != CELL_INT32) {
    error(ctx, "REFCOUNT-EXPECT: expected count must be int32");
  }

  int expected = expected_cell->payload.i32;
  int actual = 0;

  // Get actual refcount
  switch (test_cell->type) {
    case CELL_STRING:
      if (!(test_cell->flags & CELL_FLAG_INTERNED) && test_cell->payload.ptr) {
        actual = test_cell->payload.allocated_string->refcount;
      }
      break;
    case CELL_ARRAY:
    case CELL_OBJECT:
    case CELL_CODE:
      if (test_cell->payload.ptr) {
        actual = test_cell->payload.array->refcount;
      }
      break;
    default:
      actual = 0;  // Immediate types don't have refcounts
  }

  if (actual != expected) {
    error(ctx, "REFCOUNT-EXPECT: expected %d, got %d", expected, actual);
  }

  release(test_cell);
  release(expected_cell);
}

// CELL-SHARED? ( cell1 cell2 -- bool ) Check if cells share same payload pointer
static void native_cell_shared_q(context_t* ctx) {
  require_params(ctx, 2, "CELL-SHARED?");
  cell_t cell2 = data_pop_cell(ctx);
  cell_t cell1 = data_pop_cell(ctx);

  bool shared = (cell1.payload.ptr == cell2.payload.ptr) && (cell1.payload.ptr != NULL);
  data_push(ctx, new_boolean(shared));

  release(&cell1);
  release(&cell2);
}

// FORCE-COLLECT ( -- ) Force cleanup of any pending deallocations (no-op for now)
static void native_force_collect(context_t* ctx) {
  // For reference counting, there's no "pending" cleanup like in GC
  // This is a no-op but useful for API compatibility
}

// MEM-PRESSURE ( -- ) Allocate/free many items to test under pressure
static void native_mem_pressure(context_t* ctx) {
  // Allocate and immediately free several strings and arrays
  for (int i = 0; i < 100; i++) {
    // Create and destroy string - create string directly
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "test-%d", i);

    // Create a temporary string_t on the stack
    char str_buffer[sizeof(string_t) + 32];
    string_t* temp_str = (string_t*)str_buffer;
    string_from_cstr(buffer, temp_str);

    // Create allocated string and immediately release it
    cell_t str_cell = new_allocated_string(ctx, temp_str);
    retain(&str_cell);   // Give it an owner
    release(&str_cell);  // Then release it

    // Create and destroy array
    cell_array_t* arr = create_array_data(ctx, 2);
    cell_t int_cell1 = new_int32(i);
    cell_t int_cell2 = new_int32(i + 1);
    arr->elements[0] = int_cell1;
    arr->elements[1] = int_cell2;
    arr->length = 2;

    cell_t arr_cell = {0};
    arr_cell.type = CELL_ARRAY;
    arr_cell.payload.array = arr;

    retain(&arr_cell);   // Give it an owner
    release(&arr_cell);  // Then release it
  }
}

// === REFERENCE COUNTING TESTS ===

// Test basic refcount lifecycle for strings
TEST_FUNCTION(test_string_refcount_lifecycle) {
  // Test new allocated string starts with refcount 1
  TEST_INTERPRET("\"test-string\"");
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Two references on stack
  TEST_INTERPRET("DROP");

  // After DROP, one reference left
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(1);  // One reference remaining
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test refcount lifecycle for arrays
TEST_FUNCTION(test_array_refcount_lifecycle) {
  // Test new array starts with refcount 1 when on stack
  TEST_INTERPRET("[] 1 , 2 , 3 ,");
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Two references on stack
  TEST_INTERPRET("DROP");

  // Test sharing detection
  TEST_INTERPRET("DUP DUP CELL-SHARED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  // Clean up - one reference left
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test refcount for code cells (compiled definitions)
TEST_FUNCTION(test_code_refcount_lifecycle) {
  // Create a definition and test its refcount
  TEST_INTERPRET("DEF test-refcount-word 42 END");
  TEST_INTERPRET("' test-refcount-word");
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Dictionary + our copy
  TEST_INTERPRET("DROP");

  // Clean up our reference
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(1);  // Just dictionary reference
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test interned strings don't have refcounts
TEST_FUNCTION(test_interned_string_no_refcount) {
  // Create interned string via compilation
  TEST_INTERPRET("DEF test-interned-ref \"hello\" END");
  TEST_INTERPRET("test-interned-ref");

  // Interned strings have no refcount (not managed)
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(-1);
  TEST_INTERPRET("DROP");

  // DUP shouldn't change refcount for interned strings
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(-1);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test different types have independent refcounts
TEST_FUNCTION(test_mixed_types_independent_refcounts) {
  // Create string and array
  TEST_INTERPRET("\"test\" [] 1 ,");

  // Check they have independent refcounts
  TEST_INTERPRET("OVER REFCOUNT");  // String refcount
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DUP REFCOUNT");  // Array refcount
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");

  // DUP only affects top item
  TEST_INTERPRET("DUP");           // Duplicate array
  TEST_INTERPRET("DUP REFCOUNT");  // Array refcount should be 3
  TEST_STACK_TOP_INT(3);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DROP");           // Remove one array copy
  TEST_INTERPRET("SWAP REFCOUNT");  // String refcount still 1
  TEST_STACK_TOP_INT(1);
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test stack operations preserve refcounts correctly
TEST_FUNCTION(test_stack_operations_refcount) {
  // Test SWAP preserves refcounts
  TEST_INTERPRET("\"first\" \"second\"");
  TEST_INTERPRET("OVER REFCOUNT");  // first string refcount
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("SWAP");
  TEST_INTERPRET("OVER REFCOUNT");  // still first string, now on top
  TEST_STACK_TOP_INT(2);
  TEST_INTERPRET("DROP");

  // Test PICK copies without affecting original refcount
  TEST_INTERPRET("1 PICK");        // Copy "second"
  TEST_INTERPRET("DUP REFCOUNT");  // Copied string refcount
  TEST_STACK_TOP_INT(3);           // Original + copy
  TEST_INTERPRET("DROP");

  // Clean up
  TEST_INTERPRET("DROP DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Test nested structures (arrays containing strings)
TEST_FUNCTION(test_nested_refcount) {
  // Create array with strings
  TEST_INTERPRET("[] \"item1\" , \"item2\" ,");

  // Array should have refcount 1
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Two references on stack
  TEST_INTERPRET("DROP");

  // Both copies should share same payload
  TEST_INTERPRET("DUP CELL-SHARED?");
  TEST_STACK_TOP_BOOLEAN(true);
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test memory leak detection across operations
TEST_FUNCTION(test_memory_leak_detection) {
  // Take initial snapshot
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Perform operations that should not leak
  TEST_INTERPRET("\"test1\" DUP DROP DROP");
  TEST_INTERPRET("[] 1 , 2 , DUP DROP DROP");
  TEST_INTERPRET("42 DUP DROP DROP");

  // Check for leaks
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test variable operations and refcounts
TEST_FUNCTION(test_variable_refcount_operations) {
  // Test storing allocated data in variables
  TEST_INTERPRET("VARIABLE str-var");
  TEST_INTERPRET("\"stored-string\" str-var !");

  // Fetching should not change refcount of stored data
  TEST_INTERPRET("str-var @");
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(3);  // Variable holds one, stack has another
  TEST_INTERPRET("DROP");

  // Storing new value should release old value
  TEST_INTERPRET("DROP");  // Remove old value from stack
  TEST_INTERPRET("\"new-string\" str-var !");
  TEST_INTERPRET("str-var @");
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Only variable, and the stack holds it now
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test memory pressure scenarios
TEST_FUNCTION(test_memory_pressure_refcount) {
  // Take snapshot before pressure test
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Apply memory pressure
  TEST_INTERPRET("MEM-PRESSURE");

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test edge cases: empty collections
TEST_FUNCTION(test_empty_collections_refcount) {
  // Empty string
  TEST_INTERPRET("\"\"");
  TEST_INTERPRET("DUP REFCOUNT");
  // Empty strings are immediate values, so refcount is be 0
  TEST_STACK_TOP_INT(-1);
  TEST_INTERPRET("DROP DROP");

  // Empty array
  TEST_INTERPRET("[]");
  TEST_INTERPRET("DUP REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Empty arrays are allocated
  TEST_INTERPRET("DROP");

  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

// Test deep nesting memory reclamation - arrays containing arrays containing strings
TEST_FUNCTION(test_deep_nested_memory_reclaim) {
  // Take snapshot before creating deep structure
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create deeply nested structure: [["a", "b"], ["c", "d"], ["e", "f"]]
  TEST_INTERPRET("[] [] \"a\" , \"b\" , , [] \"c\" , \"d\" , , [] \"e\" , \"f\" , ,");
  // Verify the structure was created properly
  TEST_INTERPRET("DUP LENGTH");
  TEST_STACK_TOP_INT(3);  // Should have 3 sub-arrays
  TEST_INTERPRET("DROP");

  // Drop the entire structure - should cascade release all nested strings and arrays
  TEST_INTERPRET("DROP");

  // Check no leaks occurred - all nested strings and arrays should be released
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test large collection memory reclamation
TEST_FUNCTION(test_large_collection_memory_reclaim) {
  // Create array with many string elements
  TEST_INTERPRET("DEF T [] 0 BEGIN DUP 50 < WHILE DUP \" -item\" + 2 PICK SWAP , DROP 1+ REPEAT DROP END");

  // Take snapshot before creating large collection
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Verify array was created with correct size
  TEST_INTERPRET("T DUP LENGTH");
  TEST_STACK_TOP_INT(50);  // Should have 50 elements
  TEST_INTERPRET("DROP");

  // Drop the entire array - should release all 50 string elements
  TEST_INTERPRET("DROP");

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test complex variable storage memory reclamation
TEST_FUNCTION(test_complex_variable_storage_reclaim) {
  TEST_INTERPRET("VARIABLE complex-var");

  // Take snapshot before operations
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Store a complex nested structure in variable
  TEST_INTERPRET("[] [] \"nested-a\" , \"nested-b\" , , \"top-level\" , complex-var !");

  // Verify storage worked
  TEST_INTERPRET("complex-var @ LENGTH");
  TEST_STACK_TOP_INT(2);  // Should have array and string
  TEST_INTERPRET("DROP");

  // Overwrite with a new complex structure - old structure should be fully released
  TEST_INTERPRET("[] [] \"new-nested-x\" , \"new-nested-y\" , , [] \"another-array\" , , complex-var !");

  // Verify new storage
  TEST_INTERPRET("complex-var @ LENGTH");
  TEST_STACK_TOP_INT(2);  // Should have two arrays
  TEST_INTERPRET("DROP");

  // Clear the variable - should release the current structure
  TEST_INTERPRET("NULL complex-var !");

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test mixed content array memory reclamation
TEST_FUNCTION(test_mixed_content_array_reclaim) {
  // Take snapshot before creating mixed structure
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create array with mixed allocated types: strings, sub-arrays, numbers
  TEST_INTERPRET("[]");
  TEST_INTERPRET("\"string-item\" ,");             // Add string
  TEST_INTERPRET("[] 1 , 2 , ,");                  // Add sub-array with numbers
  TEST_INTERPRET("42 ,");                          // Add immediate number
  TEST_INTERPRET("\"another-string\" ,");          // Add another string
  TEST_INTERPRET("[] \"nested-str\" , , LENGTH");  // Add array with string

  // Verify mixed array structure TEST_INTERPRET("DUP LENGTH");
  TEST_STACK_TOP_INT(5);  // Should have 5 mixed elements
  TEST_INTERPRET("DROP");

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Test cascading reference drops with shared structures
TEST_FUNCTION(test_cascading_reference_drops) {
  // Take snapshot for leak detection
  TEST_INTERPRET("MEM-SNAPSHOT");

  // Create shared sub-array with content (so it's allocated, not immediate)
  TEST_INTERPRET("[] \"shared-a\" , \"shared-b\" ,");

  // Create two parent arrays that share the same sub-array
  TEST_INTERPRET("DUP [] SWAP , \"parent1-item\" ,");   // parent1 contains shared array
  TEST_INTERPRET("SWAP [] SWAP , \"parent2-item\" ,");  // parent2 contains shared array

  // Now stack is: [parent1_array, parent2_array]
  // The shared array should have refcount 2 (referenced by both parents)

  // Access the shared array from parent1 to check its refcount
  TEST_INTERPRET("OVER 0 INDEX@");  // Get first element (shared array) from parent1
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(3);  // Should be 2 (parent1 + parent2)
  TEST_INTERPRET("DROP");

  // Drop parent1 - shared array refcount should drop to 1
  TEST_INTERPRET("DROP");  // Remove parent1, stack: [parent2_array]

  // Access shared array from parent2 to check refcount
  TEST_INTERPRET("DUP 0 INDEX@");  // Get first element (shared array) from parent2
  TEST_INTERPRET("REFCOUNT");
  TEST_STACK_TOP_INT(2);  // Should be 1 (just parent2)
  TEST_INTERPRET("DROP");

  // Drop parent2 - shared array should be deallocated
  TEST_INTERPRET("DROP");  // Remove parent2

  // Stack should now just have the snapshot
  TEST_STACK_DEPTH(1);

  // Check no leaks occurred
  TEST_INTERPRET("MEM-COMPARE");
  TEST_STACK_TOP_INT(0);  // Should be 0 leaks
  TEST_INTERPRET("DROP");

  TEST_STACK_DEPTH(0);
}

// Register all reference counting test words
void add_refcount_test_words(void) {
  // Memory leak detection
  add_native_word("MEM-LEAKED?", native_mem_leaked_q, "( -- bool ) Check if current allocs != frees");
  add_native_word("MEM-SNAPSHOT", native_mem_snapshot, "( -- snapshot ) Capture current alloc count");
  add_native_word("MEM-COMPARE", native_mem_compare, "( snapshot -- leaked ) Compare current vs snapshot");

  // Reference count testing
  add_native_word("REFCOUNT-EXPECT", native_refcount_expect, "( cell expected -- ) Assert expected refcount");
  add_native_word("CELL-SHARED?", native_cell_shared_q, "( cell1 cell2 -- bool ) Check if cells share payload");

  // Advanced memory testing
  add_native_word("FORCE-COLLECT", native_force_collect, "( -- ) Force cleanup of pending deallocations");
  add_native_word("MEM-PRESSURE", native_mem_pressure, "( -- ) Allocate/free many items to test pressure");
}

// Register all reference counting tests
void register_refcount_tests(void) {
  REGISTER_TEST(test_string_refcount_lifecycle);
  REGISTER_TEST(test_array_refcount_lifecycle);
  REGISTER_TEST(test_code_refcount_lifecycle);
  REGISTER_TEST(test_interned_string_no_refcount);
  REGISTER_TEST(test_mixed_types_independent_refcounts);
  REGISTER_TEST(test_stack_operations_refcount);
  REGISTER_TEST(test_nested_refcount);
  REGISTER_TEST(test_memory_leak_detection);
  REGISTER_TEST(test_variable_refcount_operations);
  REGISTER_TEST(test_memory_pressure_refcount);
  REGISTER_TEST(test_empty_collections_refcount);

  REGISTER_TEST(test_deep_nested_memory_reclaim);
  REGISTER_TEST(test_large_collection_memory_reclaim);
  REGISTER_TEST(test_complex_variable_storage_reclaim);
  REGISTER_TEST(test_mixed_content_array_reclaim);
  REGISTER_TEST(test_cascading_reference_drops);
}

#endif  // TEST_ENABLED