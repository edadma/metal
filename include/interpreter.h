#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "metal.h"

// Core interpreter functions
metal_result_t interpret(context_t* ctx, const char* input);
void execute_code(context_t* ctx, cell_array_t* code);
bool try_parse_number(const char* token, cell_t* result);

// Error handling (tightly coupled to interpretation)
void error(const char* fmt, ...);

#endif  // INTERPRETER_H
