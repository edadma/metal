#ifndef METAL_H
#define METAL_H

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

#include "cell.h"

#ifdef TARGET_PICO
#pragma pack(pop)
#endif

#define DATA_STACK_SIZE 256
#define RETURN_STACK_SIZE 256

// Execution context (now cell_t is complete)
typedef struct context {
  // Stack management
  cell_t data_stack[DATA_STACK_SIZE];
  cell_t return_stack[RETURN_STACK_SIZE];
  int data_stack_ptr;
  int return_stack_ptr;

  // Instruction pointer (for threaded code)
  cell_t** ip;

  // Error handling
  jmp_buf error_jmp;  // For longjmp on errors
  int error_code;
  const char* error_msg;

  // Context identification
  const char* name;  // "REPL", "TIMER_IRQ", etc.
  bool is_interrupt_handler;

  // Parsing state (for words that need to parse ahead)
  const char* input_start;  // Start of input (for bounds checking/errors)
  const char* input_pos;    // Current position in input being parsed
} context_t;

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

#endif  // METAL_H