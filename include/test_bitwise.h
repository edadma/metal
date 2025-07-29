// include/test_bitwise.h
#ifndef TEST_BITWISE_H
#define TEST_BITWISE_H

#ifdef TEST_ENABLED
// Register all bitwise operation tests
void register_bitwise_tests(void);
#else
#define register_bitwise_tests() ((void)0)
#endif

#endif  // TEST_BITWISE_H