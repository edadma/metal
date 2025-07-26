#ifndef CONTEXT_H
#define CONTEXT_H

#include <setjmp.h>
#include <stdbool.h>

#include "cell.h"

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

// Context management
void init_context(context_t* ctx, const char* name);

#endif  // CONTEXT_H
