#include <ctype.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef METAL_TARGET_PICO
#include "pico/mutex.h"
#include "pico/stdlib.h"
#else
#include <pthread.h>  // Add this line for pthread functions
#endif

#include "cell.h"
#include "core.h"
#include "debug.h"
#include "dictionary.h"
#include "memory.h"
#include "metal.h"
#include "stack.h"
#include "tools.h"
#include "util.h"

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
  // Make a copy since strtok modifies the string
  char* input_copy = malloc(strlen(input) + 1);
  strcpy(input_copy, input);

  tokenize(input_copy);

  for (int i = 0; i < token_count; i++) {
    char* token = tokens[i];

    // Try to parse as number
    cell_t num;
    if (try_parse_number(token, &num)) {
      data_push(&main_context, num);
      continue;
    }

    // Try to find in dictionary
    dictionary_entry_t* word = find_word(token);
    if (word) {
      if (word->definition.type == CELL_NATIVE) {
        word->definition.payload.native(&main_context);
      } else {
        error("Non-native words not implemented yet");
      }
      continue;
    }

    // Unknown word
    printf("Unknown word: %s\n", token);
  }

  free(input_copy);
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

bool platform_get_line(char* buffer, size_t size) {
#ifdef METAL_TARGET_PICO
  // Simple character-by-character input for Pico
  size_t pos = 0;
  while (pos < size - 1) {
    int c = getchar();
    if (c == '\r' || c == '\n') {
      buffer[pos] = '\0';
      printf("\n");
      return true;
    } else if (c == '\b' || c == 127) {  // Backspace
      if (pos > 0) {
        pos--;
        printf("\b \b");
      }
    } else if (c >= 32 && c < 127) {  // Printable characters
      buffer[pos++] = c;
      putchar(c);
    }
  }
  buffer[size - 1] = '\0';
  return true;
#else
  // Use fgets on host
  if (fgets(buffer, size, stdin)) {
    // Remove newline
    buffer[strcspn(buffer, "\n")] = 0;
    return true;
  }
  return false;
#endif
}

int main(void) {
#ifdef METAL_TARGET_PICO
  stdio_init_all();
  mutex_init(&memory_mutex);

  while (!stdio_usb_connected()) {
    sleep_ms(100);
  }

  sleep_ms(500);
  printf("Metal Language v0.1 - Pico W\n");
#else
  printf("Metal Language v0.1 - Host\n");
#endif

  printf("Type 'bye' to exit, '.s' to show stack\n\n");
  printf("Cell size: %lu\n", sizeof(cell_t));

  // Initialize system
  init_memory();
  init_context(&main_context);
  init_dictionary();  // Initialize dictionary first
  populate_dictionary();

  char input[256];

  for (;;) {
    printf("\nok> ");
    fflush(stdout);

    if (!platform_get_line(input, sizeof(input))) {
      break;
    }

    interpret(input);
  }

  return 0;
}