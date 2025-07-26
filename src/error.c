#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cell.h"
#include "core.h"
#include "memory.h"

// Error handling function
void error(context_t* ctx, const char* fmt, ...) {
  static char error_buffer[256];

  // Format the error message
  va_list args;
  va_start(args, fmt);
  vsnprintf(error_buffer, sizeof(error_buffer), fmt, args);
  va_end(args);

  // Store error message in context
  ctx->error_msg = error_buffer;
  ctx->error_code = 0;

  // Clean up compilation state if error occurred during compilation
  if (compilation_mode) {
    compilation_mode = false;
    if (compiling_definition) {
      // Release all cells in the definition
      for (size_t i = 0; i < compiling_definition->length; i++) {
        metal_release(&compiling_definition->elements[i]);
      }
      metal_free(compiling_definition);
      compiling_definition = NULL;
    }
    compiling_word_name[0] = '\0';
  }

  // Clear data stack
  while (ctx->data_stack_ptr > 0) {
    ctx->data_stack_ptr--;
    metal_release(&ctx->data_stack[ctx->data_stack_ptr]);
  }

  // Clear return stack
  while (ctx->return_stack_ptr > 0) {
    ctx->return_stack_ptr--;
    metal_release(&ctx->return_stack[ctx->return_stack_ptr]);
  }

  // Clear parsing state
  ctx->input_pos = NULL;
  ctx->input_start = NULL;

  printf("ERROR: %s\n", error_buffer);

  // Jump back to error handler
  longjmp(ctx->error_jmp, 1);
}