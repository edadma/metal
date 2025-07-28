#ifndef TEST_ARITHMETIC_COMBO_H
#define TEST_ARITHMETIC_COMBO_H

#ifdef TEST_ENABLED
// Register all arithmetic combination tests
void register_arithmetic_combo_tests(void);
#else
#define register_arithmetic_combo_tests() ((void)0)
#endif

#endif  // TEST_ARITHMETIC_COMBO_H