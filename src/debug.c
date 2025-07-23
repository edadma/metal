#include "debug.h"

#ifdef DEBUG_ENABLED
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "metal.h"

// Global debug flag
bool debug_enabled = false;

void metal_debug_print(const char* file, int line, const char* fmt, ...) {
  // Extract just the filename from the full path
  const char* filename = strrchr(file, '/');
  if (filename) {
    filename++;  // Skip the '/'
  } else {
    filename = file;  // No path separator found
  }
  printf("[DEBUG %s:%d] ", filename, line);
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  printf("\n");
}

// DEBUG-ON - Enable debug output
static void native_debug_on([[maybe_unused]] context_t* ctx) {
  debug_enabled = true;
  printf("Debug output enabled\n");
}

// DEBUG-OFF - Disable debug output
static void native_debug_off([[maybe_unused]] context_t* ctx) {
  debug_enabled = false;
  printf("Debug output disabled\n");
}

// Add debug words to dictionary
void add_debug_words(void) {
  add_native_word("DEBUG-ON", native_debug_on,
                       "( -- ) Enable debug output");
  add_native_word("DEBUG-OFF", native_debug_off,
                       "( -- ) Disable debug output");
}

#endif