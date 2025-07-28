#ifndef TEST_ARITHMETIC_MIXING_H
#define TEST_ARITHMETIC_MIXING_H

#ifdef TEST_ENABLED
// Register all arithmetic type mixing tests
void register_arithmetic_mixing_tests(void);
#else
#define register_arithmetic_mixing_tests() ((void)0)
#endif

#endif  // TEST_ARITHMETIC_MIXING_H
