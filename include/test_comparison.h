#ifndef TEST_COMPARISON_H
#define TEST_COMPARISON_H

#ifdef TEST_ENABLED
// Register all comparison tests
void register_comparison_tests(void);
#else
#define register_comparison_tests() ((void)0)
#endif

#endif  // TEST_COMPARISON_H
