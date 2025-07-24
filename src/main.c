#include <ctype.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef TARGET_PICO
#include "pico/stdlib.h"
#endif

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
void error(const char* msg) {
  main_context.error_msg = msg;
  longjmp(main_context.error_jmp, 1);  // Jump back to interpret()
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

    // Clear parsing state
    main_context.input_pos = nullptr;
    main_context.input_start = nullptr;

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
      // String literal - push to stack
      data_push(&main_context, new_string(token_buffer));

    } else if (token_type == TOKEN_WORD) {
      char* word = token_buffer;

      // Try to parse as number
      cell_t num;
      if (try_parse_number(word, &num)) {
        data_push(&main_context, num);
        continue;
      }

      // Try to find in dictionary
      const dictionary_entry_t* dict_word = find_word(word);

      if (dict_word) {
        if (dict_word->definition.type == CELL_NATIVE) {
          dict_word->definition.payload.native(&main_context);
        } else {
          error("Non-native words not implemented yet");
        }
        continue;
      }

      // Unknown word
      printf("Unknown word: %s\n", word);
    }
  }

  // Clear parsing state on success
  main_context.input_pos = nullptr;
  main_context.input_start = nullptr;

  return METAL_OK;
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