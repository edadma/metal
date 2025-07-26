#include "context.h"

#include <string.h>

#include "stack.h"

context_t main_context;

// Context management
void init_context(context_t* ctx, const char* name) {
  memset(ctx, 0, sizeof(context_t));
  ctx->name = name;
  stack_init(ctx);
}