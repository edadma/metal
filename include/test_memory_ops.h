#ifndef TEST_MEMORY_OPS_H
#define TEST_MEMORY_OPS_H

#ifdef TEST_ENABLED
// Register all memory operation tests
void register_memory_ops_tests(void);
#else
#define register_memory_ops_tests() ((void)0)
#endif

#endif  // TEST_MEMORY_OPS_H