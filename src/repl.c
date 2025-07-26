#include "repl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "dictionary.h"
#include "interpreter.h"
#include "line_editor.h"
#include "stack.h"

static char input_line[INPUT_BUFFER_SIZE];

// BYE word - exit the system
void native_bye(context_t* ctx) {
  (void)ctx;

  printf("Goodbye!\n");
  exit(0);
}

// QUIT word - restart the REPL loop (simplified version)
void native_quit(context_t* ctx) {
  (void)ctx;

  // For Metal's simpler architecture, just clear stacks and continue
  ctx->data_stack_ptr = 0;
  ctx->return_stack_ptr = 0;

  printf("Restarted.\n");
}

// Enhanced REPL with line editing and history
void repl(context_t* ctx) {
  for (;;) {
    // Show appropriate prompt based on compilation state
    // Note: We'll need to add state tracking to Metal for this to work properly
    printf("\nok> ");
    fflush(stdout);

    // Get line with enhanced editing
    enhanced_get_line(input_line, INPUT_BUFFER_SIZE);

    // Skip empty lines
    if (strlen(input_line) == 0) {
      continue;
    }

    // Interpret the input
    metal_result_t result = interpret(ctx, input_line);

    // Handle any errors (Metal's simple error handling)
    if (result != METAL_OK) {
      // Error already printed by interpret(), just continue
      continue;
    }

    // Show stack depth if non-empty
    if (!is_data_empty(ctx)) {
      printf(" <%d>", data_depth(ctx));
    }
  }
}