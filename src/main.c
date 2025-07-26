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
#include "test.h"
#include "tools.h"

// Global state
context_t main_context;

// Initialize built-in words
void populate_dictionary(void) {
  add_core_words();   // Core language features
  add_tools_words();  // Development tools

  // Debug words (only when debug support compiled in)
#ifdef DEBUG_ENABLED
  add_debug_words();  // Debug commands
#endif

  // Test words (only when test support compiled in)
#ifdef TEST_ENABLED
  add_test_words();  // Test commands
  init_tests();      // Register example tests
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
  init_context(&main_context, "main");
  init_dictionary();  // Initialize dictionary first
  populate_dictionary();
  repl(&main_context);
  return 0;
}
