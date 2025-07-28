#include "core_stack.h"

#include "context.h"
#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "interpreter.h"
#include "stack.h"

// Stack manipulation words

static void native_dup(context_t* ctx) {
  debug("executing DUP");

  if (ctx->data_stack_ptr <= 0) {
    error(ctx, "DUP: stack underflow");
  }

  cell_t* top = data_peek(ctx, 0);
  data_push_ptr(ctx, top);
}

static void native_drop(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    error(ctx, "DROP: stack underflow");
  }

  cell_t* cell = data_pop(ctx);
  release(cell);
}

static void native_swap(context_t* ctx) {
  require(ctx, 2, "SWAP");

  cell_t* a = data_pop(ctx);
  cell_t b = data_pop_cell(ctx);
  data_push_ptr_no_retain(ctx, a);
  data_push_no_retain(ctx, b);
}

// PICK ( xu ... x1 x0 u -- xu ... x1 x0 xu )
// Copy the u-th item from top of stack (0-indexed)
static void native_pick(context_t* ctx) {
  require(ctx, 1, "PICK");
  cell_t* u_cell = data_pop(ctx);
  if (u_cell->type != CELL_INT32) {
    error(ctx, "PICK: index must be integer");
  }
  int u = u_cell->payload.i32;
  release(u_cell);
  if (u < 0) {
    error(ctx, "PICK: index cannot be negative");
  }
  if (u >= ctx->data_stack_ptr) {
    error(ctx, "PICK: insufficient stack");
  }
  // Copy the u-th item (0-indexed from top)
  cell_t* item = data_peek(ctx, u);
  data_push_ptr(ctx, item);
}

// ROLL ( xu xu-1 ... x1 x0 u -- xu-1 ... x1 x0 xu )
// Move the u-th item to top of stack (0-indexed)
static void native_roll(context_t* ctx) {
  require(ctx, 1, "ROLL");
  cell_t* u_cell = data_pop(ctx);
  if (u_cell->type != CELL_INT32) {
    error(ctx, "ROLL: index must be integer");
  }
  int u = u_cell->payload.i32;
  release(u_cell);
  if (u < 0) {
    error(ctx, "ROLL: index cannot be negative");
  }
  if (u == 0) {
    return;  // 0 ROLL is no-op
  }
  if (u >= ctx->data_stack_ptr) {
    error(ctx, "ROLL: insufficient stack");
  }
  // Move the u-th item to top
  int source_index = ctx->data_stack_ptr - 1 - u;
  cell_t item = ctx->data_stack[source_index];

  // Shift items down to fill the gap
  for (int i = source_index; i < ctx->data_stack_ptr - 1; i++) {
    ctx->data_stack[i] = ctx->data_stack[i + 1];
  }

  // Put the item on top
  ctx->data_stack[ctx->data_stack_ptr - 1] = item;
}

// Register core stack manipulation words
void add_core_stack_words(void) {
  // Stack manipulation
  add_native_word("DUP", native_dup, "( a -- a a ) Duplicate top of stack");
  add_native_word("DROP", native_drop, "( a -- ) Remove top of stack");
  add_native_word("SWAP", native_swap,
                  "( a b -- b a ) Swap top two stack items");
  add_native_word("PICK", native_pick,
                  "( xu...x1 x0 u -- xu...x1 x0 xu ) Copy u-th item");
  add_native_word("ROLL", native_roll,
                  "( xu...x1 x0 u -- xu-1...x1 x0 xu ) Move u-th item to top");
  add_definition("OVER", "1 PICK", "( a b -- a b a ) Copy second item to top");
  add_definition("2DUP", "OVER OVER",
                 "( a b -- a b a b ) Duplicate top two items");
  add_definition("ROT", "2 ROLL", "( a b c -- b c a ) Rotate top three items");
}