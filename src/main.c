#include <stdio.h>

#include "test.h"

#ifdef TARGET_PICO
#include "pico/stdlib.h"
#endif

#include <string.h>

#include "cell.h"
#include "context.h"
#include "core.h"
#include "core_arithmetic.h"
#include "core_array.h"
#include "core_comparison.h"
#include "core_logic.h"
#include "core_primitive.h"
#include "core_stack.h"
#include "debug.h"
#include "dictionary.h"
#include "floats.h"
#include "memory.h"
#include "repl.h"
#include "test.h"
#include "tools.h"

// Initialize built-in words
void populate_dictionary(void) {
  add_core_primitive_words();   // Primitive words
  add_core_stack_words();       // Stack manipulation
  add_core_arithmetic_words();  // Arithmetic operations
  add_core_comparison_words();  // Comparison operations
  add_core_logic_words();       // Logic operations
  add_core_array_words();       // Array operations
  add_core_words();             // Core language features
  add_tools_words();            // Development tools
  add_float_words();            // Floating-point words
  add_string_words();           // String operations

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

#ifdef TARGET_PICO
int main(void) {
#else
int main(int argc, char* argv[]) {
#endif

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
  printf("Cell size: %u\n", (uint32_t)sizeof(cell_t));

  // Initialize system
  init_memory();
  init_context(&main_context, "main");
  init_dictionary();
  populate_dictionary();

#ifdef TARGET_LINUX
  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    // intentionally run the tests twice
    run_all_tests();
    run_all_tests();
    return 0;
  }
#endif

  repl(&main_context);
  return 0;
}
