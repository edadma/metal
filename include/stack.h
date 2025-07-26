#ifndef STACK_H
#define STACK_H

#include "context.h"

// Stack initialization
void stack_init(context_t* ctx);

void require(context_t* ctx, int n, const char* op);

// Data stack operations
void data_push(context_t* ctx, cell_t cell);
void data_push_ptr(context_t* ctx, cell_t* cell);
void data_push_no_retain(context_t* ctx, cell_t cell);
void data_push_ptr_no_retain(context_t* ctx, cell_t* cell);
cell_t* data_pop(context_t* ctx);
cell_t* data_peek(context_t* ctx, int depth);
cell_t data_pop_cell(context_t* ctx);
cell_t data_peek_cell(context_t* ctx, int depth);
int data_depth(context_t* ctx);
bool is_data_empty(context_t* ctx);

// Return stack operations
void return_push(context_t* ctx, cell_t cell);
cell_t return_pop(context_t* ctx);
cell_t return_peek(context_t* ctx, int depth);
bool is_return_empty(context_t* ctx);

// Stack introspection
void print_data_stack(context_t* ctx);
void print_return_stack(context_t* ctx);

#endif  // STACK_H