#include "stack.h"

#include <stdio.h>
#include <string.h>

#include "context.h"
#include "debug.h"
#include "error.h"
#include "util.h"

// Stack initialization
void stack_init(context_t* ctx) {
  ctx->data_stack_ptr = 0;
  ctx->return_stack_ptr = 0;
  // Zero out the stacks for cleanliness
  memset(ctx->data_stack, 0, sizeof(ctx->data_stack));
  memset(ctx->return_stack, 0, sizeof(ctx->return_stack));
}

// Data stack operations
void data_push(context_t* ctx, cell_t cell) {
  data_push_no_retain(ctx, cell);

  // Retain reference if needed
  retain(&cell);
}

void data_push_no_retain(context_t* ctx, cell_t cell) {
  data_push_ptr_no_retain(ctx, &cell);
}

void data_push_ptr_no_retain(context_t* ctx, cell_t* cell) {
  if (ctx->data_stack_ptr >= DATA_STACK_SIZE) {
    error(ctx, "Data stack overflow");
    return;
  }

  debug("Pushing cell type %d to data stack (depth: %d)", cell->type,
        ctx->data_stack_ptr);

  ctx->data_stack[ctx->data_stack_ptr++] = *cell;
}

// Data stack operations
void data_push_ptr(context_t* ctx, cell_t* cell) {
  data_push_ptr_no_retain(ctx, cell);

  // Retain reference if needed
  retain(cell);
}

void require(context_t* ctx, int n, const char* op) {
  if (ctx->data_stack_ptr < n) {
    error(ctx, "%s: insufficient stack", op);
  }
}

cell_t* data_pop(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    error(ctx, "Data stack underflow");
  }

  cell_t* cell = &ctx->data_stack[--ctx->data_stack_ptr];
  debug("Popped cell type %d from data stack (depth now: %d)", cell->type,
        ctx->data_stack_ptr);

  // Note: caller is responsible for the reference now
  return cell;
}

cell_t* data_peek(context_t* ctx, int depth) {
  if (depth >= ctx->data_stack_ptr || depth < 0) {
    error(ctx, "Data stack index out of range");
  }

  return &ctx->data_stack[ctx->data_stack_ptr - 1 - depth];
}

cell_t data_pop_cell(context_t* ctx) { return *data_pop(ctx); }

cell_t data_peek_cell(context_t* ctx, int depth) {
  return *data_peek(ctx, depth);
}

int data_depth(context_t* ctx) { return ctx->data_stack_ptr; }

bool is_data_empty(context_t* ctx) { return ctx->data_stack_ptr == 0; }

// Return stack operations
void return_push(context_t* ctx, cell_t cell) {
  if (ctx->return_stack_ptr >= RETURN_STACK_SIZE) {
    error(ctx, "Return stack overflow");
    return;
  }

  debug("Pushing cell type %d to return stack (depth: %d)", cell.type,
        ctx->return_stack_ptr);

  // Retain reference if needed
  retain(&cell);

  ctx->return_stack[ctx->return_stack_ptr++] = cell;
}

cell_t* return_pop(context_t* ctx) {
  if (ctx->return_stack_ptr <= 0) {
    error(ctx, "Return stack underflow");
  }

  cell_t* cell = &ctx->return_stack[--ctx->return_stack_ptr];

  debug("Popped cell type %d from return stack (depth now: %d)", cell->type,
        ctx->return_stack_ptr);

  // Note: caller is responsible for the reference now
  return cell;
}

cell_t return_pop_cell(context_t* ctx) {
  if (ctx->return_stack_ptr <= 0) {
    error(ctx, "Return stack underflow");
    return new_empty();
  }

  cell_t cell = ctx->return_stack[--ctx->return_stack_ptr];
  debug("Popped cell type %d from return stack (depth now: %d)", cell.type,
        ctx->return_stack_ptr);

  // Note: caller is responsible for the reference now
  return cell;
}

cell_t return_peek(context_t* ctx, int depth) {
  if (depth >= ctx->return_stack_ptr || depth < 0) {
    error(ctx, "Return stack index out of range");
    return new_empty();
  }

  return ctx->return_stack[ctx->return_stack_ptr - 1 - depth];
}

bool is_return_empty(context_t* ctx) { return ctx->return_stack_ptr == 0; }

// Stack introspection
void print_data_stack(context_t* ctx) {
  printf("Data Stack (%d): ", ctx->data_stack_ptr);
  for (int i = 0; i < ctx->data_stack_ptr; i++) {
    if (i > 0) printf(" ");
    print_cell(&ctx->data_stack[i]);
  }
  printf("\n");
}

void print_return_stack(context_t* ctx) {
  printf("Return Stack (%d): ", ctx->return_stack_ptr);
  for (int i = 0; i < ctx->return_stack_ptr; i++) {
    if (i > 0) printf(" ");
    print_cell(&ctx->return_stack[i]);
  }
  printf("\n");
}