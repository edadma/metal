#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "context.h"

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
void execute_code(context_t* ctx, cell_array_t* code);
bool try_parse_number(const char* token, cell_t* result);
void compile_cell(cell_t cell);

#endif  // INTERPRETER_H
