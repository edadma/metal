#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "context.h"
#include "dictionary.h"

// Interpreter result codes
typedef enum {
  METAL_OK,
  METAL_ERROR,
  METAL_STACK_UNDERFLOW,
  METAL_STACK_OVERFLOW,
  METAL_COMPILE_ERROR,
} metal_result_t;

extern bool compilation_mode;
extern cell_array_t* compiling_definition;
extern char compiling_word_name[MAX_NAME_LENGTH];

void add_definition(const char* name, const char* source, const char* help);
metal_result_t interpret(context_t* ctx, bool print_errors, const char* input);
void execute_code(context_t* ctx);
bool try_parse_number(context_t* ctx, const char* token, cell_t* result);
void compile_cell(context_t* ctx, cell_t cell);

void reset_interpreter_state(void);

#endif  // INTERPRETER_H
