#ifndef TEST_H
#define TEST_H

#include <stdbool.h>

#include "context.h"

#ifdef TEST_ENABLED

// Test statistics
extern int test_count;
extern int test_passed;
extern int test_failed;

// Core test macros - capture file/line/expression for good error reporting
#define TEST_ASSERT(cond) test_assert(__FILE__, __LINE__, #cond, (cond))

#define TEST_EQUAL(a, b) test_equal(__FILE__, __LINE__, #a " == " #b, (a), (b))

#define TEST_NOT_EQUAL(a, b) \
  test_not_equal(__FILE__, __LINE__, #a " != " #b, (a), (b))

#define TEST_NULL(ptr) test_null(__FILE__, __LINE__, #ptr, (ptr))

#define TEST_NOT_NULL(ptr) test_not_null(__FILE__, __LINE__, #ptr, (ptr))

#define TEST_STRING_EQUAL(a, b) \
  test_string_equal(__FILE__, __LINE__, #a " == " #b, (a), (b))

// Metal language testing macros
#define TEST_INTERPRET(code) test_interpret(__FILE__, __LINE__, code)

#define TEST_STACK_DEPTH(expected) \
  test_stack_depth(__FILE__, __LINE__, #expected, (expected))

#define TEST_STACK_TOP_INT(expected) \
  test_stack_top_int(__FILE__, __LINE__, #expected, (expected))

#define TEST_STACK_TOP_FLOAT(expected) \
  test_stack_top_float(__FILE__, __LINE__, #expected, (expected))

#define TEST_STACK_TOP_STRING(expected) \
  test_stack_top_string(__FILE__, __LINE__, #expected, (expected))

// Test function registration
#define TEST_FUNCTION(name) static void test_##name(void)
#define REGISTER_TEST(name) register_test(#name, test_##name)

// Test implementation functions
void test_assert(const char* file, int line, const char* expr, bool condition);
void test_equal(const char* file, int line, const char* expr, int a, int b);
void test_not_equal(const char* file, int line, const char* expr, int a, int b);
void test_null(const char* file, int line, const char* expr, void* ptr);
void test_not_null(const char* file, int line, const char* expr, void* ptr);
void test_string_equal(const char* file, int line, const char* expr,
                       const char* a, const char* b);

// Metal language testing functions
void test_interpret(const char* file, int line, const char* code);
void test_stack_depth(const char* file, int line, const char* expr,
                      int expected);
void test_stack_top_int(const char* file, int line, const char* expr,
                        int expected);
void test_stack_top_float(const char* file, int line, const char* expr,
                          double expected);
void test_stack_top_string(const char* file, int line, const char* expr,
                           const char* expected);

// Test management
void register_test(const char* name, void (*test_func)(void));
void run_all_tests(void);
void reset_test_stats(void);
context_t* get_test_context(void);

// Add test words to dictionary
void add_test_words(void);

void init_tests(void);
#else
// No-op macros when testing disabled
#define TEST_ASSERT(cond) ((void)0)
#define TEST_EQUAL(a, b) ((void)0)
#define TEST_NOT_EQUAL(a, b) ((void)0)
#define TEST_NULL(ptr) ((void)0)
#define TEST_NOT_NULL(ptr) ((void)0)
#define TEST_STRING_EQUAL(a, b) ((void)0)
#define TEST_INTERPRET(code) ((void)0)
#define TEST_STACK_DEPTH(expected) ((void)0)
#define TEST_STACK_TOP_INT(expected) ((void)0)
#define TEST_STACK_TOP_FLOAT(expected) ((void)0)
#define TEST_STACK_TOP_STRING(expected) ((void)0)
#define TEST_FUNCTION(name) \
  static void test_##name(void) {}
#define REGISTER_TEST(name) ((void)0)
#define add_test_words() ((void)0)
#endif

#endif  // TEST_H
