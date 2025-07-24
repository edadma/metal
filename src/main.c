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
  printf("ERROR: %s\n", msg);
  // For now, just continue. Later we'll use longjmp
}

// Simple tokenizer
#define MAX_TOKENS 64
static char* tokens[MAX_TOKENS];
static int token_count = 0;

void tokenize(char* input) {
  token_count = 0;
  char* token = strtok(input, " \t\n\r");

  while (token && token_count < MAX_TOKENS) {
    tokens[token_count++] = token;
    token = strtok(NULL, " \t\n\r");
  }
}

// Number parsing
bool try_parse_number(const char* token, cell_t* result) {
  char* endptr;

  // Try integer first
  long long val = strtoll(token, &endptr, 10);
  if (*endptr == '\0') {
    if (val >= INT32_MIN && val <= INT32_MAX) {
      *result = new_int32((int32_t)val);
    } else {
      *result = new_int64(val);
    }
    return true;
  }

  // Try float
  double fval = strtod(token, &endptr);
  if (*endptr == '\0') {
    *result = new_float(fval);
    return true;
  }

  return false;
}

// Main interpreter
metal_result_t interpret(const char* input) {
  // Set up parsing state in context
  main_context.input_start = input;
  main_context.input_pos = input;
  char word_buffer[256];
  // Parse and execute words one at a time
  while (parse_next_word(&main_context.input_pos, word_buffer,
                         sizeof(word_buffer))) {
    char* word = word_buffer;
    // Try to parse as number
    cell_t num;
    if (try_parse_number(word, &num)) {
      data_push(&main_context, num);
      continue;
    }
    // Try to find in dictionary
    dictionary_entry_t* dict_word = find_word(word);
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

  // Clear parsing state
  main_context.input_pos = NULL;
  main_context.input_start = NULL;

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