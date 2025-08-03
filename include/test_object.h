// Add to include/test_object.h (new file)
#ifndef TEST_OBJECT_H
#define TEST_OBJECT_H

#ifdef TEST_ENABLED

// Register all object manipulation tests
void register_object_tests(void);

#endif  // TEST_ENABLED

#endif  // TEST_OBJECT_H