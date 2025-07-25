#include "test.h"

#ifdef TEST_ENABLED
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "stack.h"
#include "util.h"

// Test statistics
int test_count = 0;
int test_passed = 0;
int test_failed = 0;

// Test registry
#define MAX_TESTS 100
static struct {
  const char* name;
  void (*func)(void);
} test_registry[MAX_TESTS];
static int registered_test_count = 0;

// Test context for Metal language testing
static context_t test_context;
static bool test_context_initialized = false;

// Helper to get filename from path
static const char* get_filename(const char* path) {
  const char* filename = strrchr(path, '/');
  if (filename) {
    return filename + 1;
  }
  filename = strrchr(path, '\\');  // Windows path separator
  if (filename) {
    return filename + 1;
  }
  return path;
}

// Helper to print test failure
static void test_fail(const char* file, int line, const char* expr) {
  printf("FAIL: %s:%d - %s\n", get_filename(file), line, expr);
  test_failed++;
}

// Helper to print test pass
static void test_pass(const char* file, int line, const char* expr) {
  printf("PASS: %s:%d - %s\n", get_filename(file), line, expr);
  test_passed++;
}

// Core test functions
void test_assert(const char* file, int line, const char* expr, bool condition) {
  test_count++;
  if (condition) {
    test_pass(file, line, expr);
  } else {
    test_fail(file, line, expr);
  }
}

void test_equal(const char* file, int line, const char* expr, int a, int b) {
  test_count++;
  if (a == b) {
    test_pass(file, line, expr);
  } else {
    printf("FAIL: %s:%d - %s (got %d, expected %d)\n", get_filename(file), line,
           expr, a, b);
    test_failed++;
  }
}

void test_not_equal(const char* file, int line, const char* expr, int a,
                    int b) {
  test_count++;
  if (a != b) {
    test_pass(file, line, expr);
  } else {
    printf("FAIL: %s:%d - %s (both values are %d)\n", get_filename(file), line,
           expr, a);
    test_failed++;
  }
}

void test_null(const char* file, int line, const char* expr, void* ptr) {
  test_count++;
  if (ptr == NULL) {
    test_pass(file, line, expr);
  } else {
    printf("FAIL: %s:%d - %s (got %p, expected NULL)\n", get_filename(file),
           line, expr, ptr);
    test_failed++;
  }
}

void test_not_null(const char* file, int line, const char* expr, void* ptr) {
  test_count++;
  if (ptr != NULL) {
    test_pass(file, line, expr);
  } else {
    test_fail(file, line, expr);
  }
}

void test_string_equal(const char* file, int line, const char* expr,
                       const char* a, const char* b) {
  test_count++;
  if (a && b && strcmp(a, b) == 0) {
    test_pass(file, line, expr);
  } else {
    printf("FAIL: %s:%d - %s (got \"%s\", expected \"%s\")\n",
           get_filename(file), line, expr, a ? a : "(null)", b ? b : "(null)");
    test_failed++;
  }
}

// Metal language testing functions
context_t* get_test_context(void) {
  if (!test_context_initialized) {
    init_context(&test_context);
    test_context.name = "test";
    test_context_initialized = true;
  }
  return &test_context;
}

void test_interpret(const char* file, int line, const char* code) {
  test_count++;
  context_t* ctx = get_test_context();

  // Clear any previous error state
  ctx->error_msg = NULL;

  metal_result_t result = interpret(code);
  if (result == METAL_OK) {
    printf("PASS: %s:%d - interpret(\"%s\")\n", get_filename(file), line, code);
    test_passed++;
  } else {
    printf("FAIL: %s:%d - interpret(\"%s\") failed: %s\n", get_filename(file),
           line, code, ctx->error_msg ? ctx->error_msg : "unknown error");
    test_failed++;
  }
}

void test_stack_depth(const char* file, int line, const char* expr,
                      int expected) {
  test_count++;
  context_t* ctx = get_test_context();
  int actual = data_depth(ctx);

  if (actual == expected) {
    test_pass(file, line, expr);
  } else {
    printf("FAIL: %s:%d - %s (got depth %d, expected %d)\n", get_filename(file),
           line, expr, actual, expected);
    test_failed++;
  }
}

void test_stack_top_int(const char* file, int line, const char* expr,
                        int expected) {
  test_count++;
  context_t* ctx = get_test_context();

  if (is_data_empty(ctx)) {
    printf("FAIL: %s:%d - %s (stack is empty)\n", get_filename(file), line,
           expr);
    test_failed++;
    return;
  }

  cell_t top = data_peek(ctx, 0);
  if (top.type == CELL_INT32 && top.payload.i32 == expected) {
    test_pass(file, line, expr);
  } else if (top.type == CELL_INT32) {
    printf("FAIL: %s:%d - %s (got %d, expected %d)\n", get_filename(file), line,
           expr, top.payload.i32, expected);
    test_failed++;
  } else {
    printf("FAIL: %s:%d - %s (top is not int32, type=%d)\n", get_filename(file),
           line, expr, top.type);
    test_failed++;
  }
}

// void test_stack_top_float(const char* file, int line, const char* expr,
//                           double expected) {
//   test_count++;
//   context_t* ctx = get_test_context();
//
//   if (is_data_empty(ctx)) {
//     printf("FAIL: %s:%d - %s (stack is empty)\n", get_filename(file), line,
//            expr);
//     test_failed++;
//     return;
//   }
//
//   cell_t top = data_peek(ctx, 0);
//   if (top.type == CELL_FLOAT && fabs(top.payload.f - expected) < 1e-10) {
//     test_pass(file, line, expr);
//   } else if (top.type == CELL_FLOAT) {
//     printf("FAIL: %s:%d - %s (got %g, expected %g)\n", get_filename(file),
//     line,
//            expr, top.payload.f, expected);
//     test_failed++;
//   } else {
//     printf("FAIL: %s:%d - %s (top is not float, type=%d)\n",
//     get_filename(file),
//            line, expr, top.type);
//     test_failed++;
//   }
// }

void test_stack_top_string(const char* file, int line, const char* expr,
                           const char* expected) {
  test_count++;
  context_t* ctx = get_test_context();

  if (is_data_empty(ctx)) {
    printf("FAIL: %s:%d - %s (stack is empty)\n", get_filename(file), line,
           expr);
    test_failed++;
    return;
  }

  cell_t top = data_peek(ctx, 0);
  if (top.type == CELL_STRING) {
    const char* actual = (const char*)top.payload.ptr;
    if (actual && expected && strcmp(actual, expected) == 0) {
      test_pass(file, line, expr);
    } else {
      printf("FAIL: %s:%d - %s (got \"%s\", expected \"%s\")\n",
             get_filename(file), line, expr, actual ? actual : "(null)",
             expected);
      test_failed++;
    }
  } else {
    printf("FAIL: %s:%d - %s (top is not string, type=%d)\n",
           get_filename(file), line, expr, top.type);
    test_failed++;
  }
}

// Test management
void register_test(const char* name, void (*test_func)(void)) {
  if (registered_test_count >= MAX_TESTS) {
    printf("ERROR: Too many tests registered (max %d)\n", MAX_TESTS);
    return;
  }

  test_registry[registered_test_count].name = name;
  test_registry[registered_test_count].func = test_func;
  registered_test_count++;
}

void reset_test_stats(void) {
  test_count = 0;
  test_passed = 0;
  test_failed = 0;

  // Clear test context stacks
  if (test_context_initialized) {
    context_t* ctx = get_test_context();
    while (!is_data_empty(ctx)) {
      cell_t cell = data_pop(ctx);
      metal_release(&cell);
    }
    while (!is_return_empty(ctx)) {
      cell_t cell = return_pop(ctx);
      metal_release(&cell);
    }
  }
}

void run_all_tests(void) {
  printf("\n=== Running Metal Unit Tests ===\n");
  reset_test_stats();

  for (int i = 0; i < registered_test_count; i++) {
    printf("\n--- Test: %s ---\n", test_registry[i].name);
    test_registry[i].func();
  }

  printf("\n=== Test Results ===\n");
  printf("Total tests: %d\n", test_count);
  printf("Passed: %d\n", test_passed);
  printf("Failed: %d\n", test_failed);

  if (test_failed == 0) {
    printf("All tests PASSED! ✓\n");
  } else {
    printf("%d tests FAILED! ✗\n", test_failed);
  }
}

// TEST word - run all registered tests
static void native_test(context_t* ctx) {
  (void)ctx;  // Unused parameter
  run_all_tests();
}

// Add test words to dictionary
void add_test_words(void) {
  add_native_word("TEST", native_test, "( -- ) Run all unit tests");
}

// Example test functions to demonstrate usage
TEST_FUNCTION(basic_arithmetic) {
  TEST_INTERPRET("5 3 +");
  TEST_STACK_DEPTH(1);
  TEST_STACK_TOP_INT(8);
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(string_operations) {
  TEST_INTERPRET("\"hello\" \"world\"");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_STRING("world");
  TEST_INTERPRET("DROP");
  TEST_STACK_TOP_STRING("hello");
  TEST_INTERPRET("DROP");
  TEST_STACK_DEPTH(0);
}

TEST_FUNCTION(array_operations) {
  TEST_INTERPRET("[] 1 , 2 , 3 ,");
  TEST_STACK_DEPTH(1);

  TEST_INTERPRET("DUP LENGTH");
  TEST_STACK_DEPTH(2);
  TEST_STACK_TOP_INT(3);

  TEST_INTERPRET("DROP DROP");
  TEST_STACK_DEPTH(0);
}

// Register example tests (would be called from main or test initialization)
static void register_example_tests(void) {
  REGISTER_TEST(basic_arithmetic);
  REGISTER_TEST(string_operations);
  REGISTER_TEST(array_operations);
}

// Call this from main.c when TEST_ENABLED
void init_tests(void) { register_example_tests(); }

#endif  // TEST_ENABLED