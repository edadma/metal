#ifndef TEST_COMPLEX_ARITHMETIC_H
#define TEST_COMPLEX_ARITHMETIC_H

#ifdef TEST_ENABLED
// Register all complex arithmetic tests
void register_complex_arithmetic_tests(void);
#else
#define register_complex_arithmetic_tests() ((void)0)
#endif

#endif  // TEST_COMPLEX_ARITHMETIC_H