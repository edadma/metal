#ifndef TEST_REFCOUNT_H
#define TEST_REFCOUNT_H

#ifdef TEST_ENABLED
void add_refcount_test_words(void);
void register_refcount_tests(void);
#else
#define add_refcount_test_words() ((void)0)
#define register_refcount_tests() ((void)0)
#endif

#endif  // TEST_REFCOUNT_H