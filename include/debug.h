#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>

#ifdef DEBUG_ENABLED
// Runtime debug flag
extern bool debug_enabled;
#define debug(fmt, ...)                                          \
  do {                                                           \
    if (debug_enabled) {                                         \
      metal_debug_print(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    }                                                            \
  } while (0)

void metal_debug_print(const char* file, int line, const char* fmt, ...);
void add_debug_words(void);  // Function to add debug words to dictionary
#else
#define debug(fmt, ...) ((void)0)
#define add_debug_words() ((void)0)  // No-op when debug disabled
#endif

#endif  // DEBUG_H