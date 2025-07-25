#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TARGET_PICO
#include "pico/stdlib.h"
#endif

#include "array.h"
#include "cell.h"
#include "core.h"
#include "debug.h"
#include "dictionary.h"
#include "memory.h"
#include "metal.h"
#include "parser.h"
#include "repl.h"
#include "stack.h"
#include "tools.h"

// Global state
static context_t main_context;

// Context management
void init_context(context_t* ctx) {
  memset(ctx, 0, sizeof(context_t));
  ctx->name = "main";
}

// Error handling
void error(const char* fmt, ...) {
  static char error_buffer[256];  // Static buffer for formatted message

  va_list args;
  va_start(args, fmt);
  vsnprintf(error_buffer, sizeof(error_buffer), fmt, args);
  va_end(args);

  // Clear stacks before jumping (releases all references)
  while (!is_data_empty(&main_context)) {
    cell_t cell = data_pop(&main_context);
    metal_release(&cell);
  }

  while (!is_return_empty(&main_context)) {
    cell_t cell = return_pop(&main_context);
    metal_release(&cell);
  }

  main_context.error_msg = error_buffer;
  longjmp(main_context.error_jmp, 1);
}

// Number parsing
bool try_parse_number(const char* token, cell_t* result) {
  char* endptr;

  // Try integer first
  const long long val = strtoll(token, &endptr, 10);

  if (*endptr == '\0') {
    if (val >= INT32_MIN && val <= INT32_MAX) {
      *result = new_int32((int32_t)val);
    } else {
      *result = new_int64(val);
    }
    return true;
  }

  // Try float
  const double fval = strtod(token, &endptr);

  if (*endptr == '\0') {
    *result = new_float(fval);
    return true;
  }

  return false;
}

// Main interpreter
metal_result_t interpret(const char* input) {
  // Set up exception handling
  if (setjmp(main_context.error_jmp) != 0) {
    // We jumped here due to an error
    printf("ERROR: %s\n", main_context.error_msg);

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

    // Clear parsing state
    main_context.input_pos = NULL;
    main_context.input_start = NULL;

    return METAL_ERROR;
  }

  // Set up parsing state in context
  main_context.input_start = input;
  main_context.input_pos = input;

  char token_buffer[256];
  token_type_t token_type;

  // Parse and execute tokens one at a time
  while ((token_type = parse_next_token(&main_context.input_pos, token_buffer,
                                        sizeof(token_buffer))) != TOKEN_EOF) {
    if (token_type == TOKEN_STRING) {
      // String literal
      cell_t string_cell = new_string(token_buffer);

      if (compilation_mode) {
        compile_cell(string_cell);
      } else {
        data_push(&main_context, string_cell);
      }

    } else if (token_type == TOKEN_WORD) {
      char* word = token_buffer;

      // Try to parse as number first
      cell_t num;
      if (try_parse_number(word, &num)) {
        if (compilation_mode) {
          compile_cell(num);
        } else {
          data_push(&main_context, num);
        }
        continue;
      }

      // Try to find in dictionary
      const dictionary_entry_t* dict_word = find_word(word);

      if (dict_word) {
        // Check if word is immediate (executes even during compilation)
        bool is_immediate =
            (dict_word->definition.flags & CELL_FLAG_IMMEDIATE) != 0;

        if (compilation_mode && !is_immediate) {
          // Compile the word reference
          compile_cell(dict_word->definition);
        } else {
          // Execute the word (either interpretation mode or immediate word)
          if (dict_word->definition.type == CELL_NATIVE) {
            dict_word->definition.payload.native(&main_context);
          } else if (dict_word->definition.type == CELL_CODE) {
            execute_code(&main_context,
                         (array_data_t*)dict_word->definition.payload.ptr);
          } else {
            error("Unknown word type: %d", dict_word->definition.type);
          }
        }
        continue;
      }

      // Unknown word
      error("Unknown word: %s", word);
    }
  }

  // Clear parsing state on success
  main_context.input_pos = NULL;
  main_context.input_start = NULL;

  return METAL_OK;
}

void execute_code(context_t* ctx, array_data_t* code) {
  for (size_t i = 0; i < code->length; i++) {
    cell_t* cell = &code->elements[i];
    switch (cell->type) {
      case CELL_NATIVE: {
        // Execute native function
        cell->payload.native(ctx);
        break;
      }

      case CELL_CODE: {
        // Recursive call to execute nested code
        execute_code(ctx, (array_data_t*)cell->payload.ptr);
        break;
      }

        // All other cell types push themselves onto the stack
      case CELL_INT32:
      case CELL_INT64:
      case CELL_FLOAT:
      case CELL_STRING:
      case CELL_ARRAY:
      case CELL_NIL:
      case CELL_EMPTY:
      default: {
        cell_t copy = *cell;
        metal_retain(&copy);
        data_push(ctx, copy);
        break;
      }
    }
  }
}

// Initialize built-in words
void populate_dictionary(void) {
  add_core_words();   // Core language features
  add_tools_words();  // Development tools

  // Debug words (only when debug support compiled in)
#ifdef DEBUG_ENABLED
  add_debug_words();  // Debug commands
#endif
}

int main(void) {
#ifdef TARGET_PICO
  stdio_init_all();

  while (!stdio_usb_connected()) {
    sleep_ms(100);
  }

  sleep_ms(500);
#define TARGET "Pico W"
#elifdef TARGET_LINUX
#define TARGET "Linux"
#elifdef TARGET_WINDOWS
#define TARGET "Windows"
#endif

  printf("Metal Language v" METAL_VERSION " - " TARGET "\n");
  printf("Type 'bye' to exit, '.s' to show stack\n\n");
  printf("Cell size: %lu\n", sizeof(cell_t));

  // Initialize system
  init_memory();
  init_context(&main_context);
  init_dictionary();  // Initialize dictionary first
  populate_dictionary();
  repl(&main_context);
  return 0;
}