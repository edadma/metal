#ifndef TEST_VARIABLES_H
#define TEST_VARIABLES_H

#ifdef TEST_ENABLED
// Register all variables and constants tests
void register_variables_tests(void);
#else
#define register_variables_tests() ((void)0)
#endif

#endif  // TEST_VARIABLES_H