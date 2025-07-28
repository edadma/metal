#ifndef TEST_CONVERSION_H
#define TEST_CONVERSION_H

#ifdef TEST_ENABLED
// Register all type conversion tests
void register_conversion_tests(void);
#else
#define register_conversion_tests() ((void)0)
#endif

#endif  // TEST_CONVERSION_H