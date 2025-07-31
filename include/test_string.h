#ifndef TEST_STRING_H
#define TEST_STRING_H

#ifdef TEST_ENABLED
void register_string_tests(void);
#else
#define register_string_tests() ((void)0)
#endif

#endif  // TEST_STRING_H