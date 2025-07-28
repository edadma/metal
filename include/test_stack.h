#ifndef TEST_STACK_H
#define TEST_STACK_H

#ifdef TEST_ENABLED
// Register all stack manipulation tests
void register_stack_tests(void);
#else
#define register_stack_tests() ((void)0)
#endif

#endif  // TEST_STACK_H