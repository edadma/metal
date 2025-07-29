#ifndef TEST_LOGIC_H
#define TEST_LOGIC_H

#ifdef TEST_ENABLED
// Register all logic operation tests
void register_logic_tests(void);
#else
#define register_logic_tests() ((void)0)
#endif

#endif  // TEST_LOGIC_H