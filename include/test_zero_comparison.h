#ifndef TEST_ZERO_COMPARISON_H
#define TEST_ZERO_COMPARISON_H

#ifdef TEST_ENABLED
// Register all zero comparison tests
void register_zero_comparison_tests(void);
#else
#define register_zero_comparison_tests() ((void)0)
#endif

#endif  // TEST_ZERO_COMPARISON_H