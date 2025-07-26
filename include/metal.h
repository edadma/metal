#ifndef METAL_H
#define METAL_H

#include <stdbool.h>
#include <stdint.h>

#include "cell.h"

#ifdef TARGET_PICO
#pragma pack(pop)
#endif

// Allocated data header (for refcounting)
typedef struct {
  uint32_t refcount;
  // Actual data follows
} alloc_header_t;

// Dictionary entry
typedef struct {
  char name[32];      // Word name
  cell_t definition;  // Code cell or other definition
  const char* help;   // Help text (stack effect + description)
} dictionary_entry_t;

// Interpreter result codes
typedef enum {
  METAL_OK,
  METAL_ERROR,
  METAL_STACK_UNDERFLOW,
  METAL_STACK_OVERFLOW,
  METAL_COMPILE_ERROR,
} metal_result_t;

// Core interpreter functions
metal_result_t interpret(context_t* ctx, const char* input);
bool metal_input_complete(const char* input);
void error(const char* fmt, ...);

// Context management
void init_context(context_t* ctx, const char* name);
void metal_switch_context(context_t* ctx);

// Compatibility for unused parameters
#if defined(__GNUC__) || defined(__clang__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#endif  // METAL_H