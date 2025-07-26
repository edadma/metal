#ifndef METAL_H
#define METAL_H

#include <stdbool.h>
#include <stdint.h>

// Compatibility for unused parameters
#if defined(__GNUC__) || defined(__clang__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#endif  // METAL_H