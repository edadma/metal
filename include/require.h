#ifndef REQUIRE_H
#define REQUIRE_H

#include "error.h"

// require macro - like assert() but calls error() instead of abort()
// Usage: require(ctx, ptr != NULL);
#define require(ctx, condition)                                         \
  do {                                                                  \
    if (!(condition)) {                                                 \
      error(ctx, "Assertion failed: %s at %s:%d", #condition, __FILE__, \
            __LINE__);                                                  \
    }                                                                   \
  } while (0)

// require_msg - same as require but with custom error message and file/line
// info Usage: require_msg(ctx, ptr != NULL, "Pointer cannot be NULL in function
// %s", __func__);
#define require_msg(ctx, condition, msg, ...)                        \
  do {                                                               \
    if (!(condition)) {                                              \
      error(ctx, msg " [%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__); \
    }                                                                \
  } while (0)

#endif  // REQUIRE_H