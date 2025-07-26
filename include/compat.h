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

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <stdnoreturn.h>
#define NORETURN noreturn
#elif defined(__GNUC__) || defined(__clang__)
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif
