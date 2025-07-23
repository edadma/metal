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

#include "core.h"
#include "debug.h"
#include "metal.h"
#include "stack.h"
#include "util.h"

// Global state
static context_t main_context;

// Simple dictionary - just a linear array
#define MAX_DICT_ENTRIES 256
static dict_entry_t dictionary[MAX_DICT_ENTRIES];
static int dict_size = 0;

#ifdef METAL_TARGET_PICO
static mutex_t memory_mutex;
#else
static pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

// Memory management
void* metal_alloc(size_t size) {
  debug("Allocating %zu bytes", size);

#ifdef METAL_TARGET_PICO
  mutex_enter_blocking(&memory_mutex);
#else
  pthread_mutex_lock(&memory_mutex);
#endif

  alloc_header_t* header = malloc(sizeof(alloc_header_t) + size);
  if (!header) {
    error("Out of memory");
#ifdef METAL_TARGET_PICO
    mutex_exit(&memory_mutex);
#else
    pthread_mutex_unlock(&memory_mutex);
#endif
    return NULL;
  }
  header->refcount = 1;

#ifdef METAL_TARGET_PICO
  mutex_exit(&memory_mutex);
#else
  pthread_mutex_unlock(&memory_mutex);
#endif

  return (char*)header + sizeof(alloc_header_t);
}

void* metal_realloc(void* ptr, size_t new_size) {
#ifdef METAL_TARGET_PICO
  mutex_enter_blocking(&memory_mutex);
#else
  pthread_mutex_lock(&memory_mutex);
#endif

  if (!ptr) {
    // Just allocate new
    alloc_header_t* header = malloc(sizeof(alloc_header_t) + new_size);
    if (!header) {
      error("Out of memory");
#ifdef METAL_TARGET_PICO
      mutex_exit(&memory_mutex);
#else
      pthread_mutex_unlock(&memory_mutex);
#endif
      return NULL;
    }
    header->refcount = 1;
#ifdef METAL_TARGET_PICO
    mutex_exit(&memory_mutex);
#else
    pthread_mutex_unlock(&memory_mutex);
#endif
    return (char*)header + sizeof(alloc_header_t);
  }

  alloc_header_t* old_header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  alloc_header_t* new_header =
      realloc(old_header, sizeof(alloc_header_t) + new_size);

  if (!new_header) {
    error("Out of memory");
#ifdef METAL_TARGET_PICO
    mutex_exit(&memory_mutex);
#else
    pthread_mutex_unlock(&memory_mutex);
#endif
    return NULL;
  }

#ifdef METAL_TARGET_PICO
  mutex_exit(&memory_mutex);
#else
  pthread_mutex_unlock(&memory_mutex);
#endif

  return (char*)new_header + sizeof(alloc_header_t);
}

void metal_free(void* ptr) {
  if (!ptr) return;

#ifdef METAL_TARGET_PICO
  mutex_enter_blocking(&memory_mutex);
#else
  pthread_mutex_lock(&memory_mutex);
#endif

  alloc_header_t* header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  free(header);

#ifdef METAL_TARGET_PICO
  mutex_exit(&memory_mutex);
#else
  pthread_mutex_unlock(&memory_mutex);
#endif
}

// Updated retain/release functions to handle CELL_ARRAY and CELL_POINTER
void metal_retain(cell_t* cell) {
  if (!cell || !cell->payload.ptr) return;

  // Only allocated types need refcount management
  switch (cell->type) {
    case CELL_STRING:
    case CELL_OBJECT:
    case CELL_ARRAY:
    case CELL_CODE:
      if (!(cell->flags & CELL_FLAG_WEAK_REF)) {
        alloc_header_t* header = (alloc_header_t*)((char*)cell->payload.ptr -
                                                   sizeof(alloc_header_t));
        header->refcount++;
      }
      break;
    case CELL_POINTER:
      // For pointers, we don't manage the pointed-to memory's refcount
      // The pointer itself doesn't own the memory
      break;
    default:
      break;
  }
}

void metal_release(cell_t* cell) {
  if (!cell || !cell->payload.ptr) return;

  switch (cell->type) {
    case CELL_STRING:
    case CELL_OBJECT:
    case CELL_CODE:
      if (!(cell->flags & CELL_FLAG_WEAK_REF)) {
        alloc_header_t* header = (alloc_header_t*)((char*)cell->payload.ptr -
                                                   sizeof(alloc_header_t));
        header->refcount--;
        if (header->refcount == 0) {
          metal_free(cell->payload.ptr);
          cell->payload.ptr = NULL;
        }
      }
      break;
    case CELL_ARRAY:
      if (!(cell->flags & CELL_FLAG_WEAK_REF)) {
        alloc_header_t* header = (alloc_header_t*)((char*)cell->payload.ptr -
                                                   sizeof(alloc_header_t));
        header->refcount--;
        if (header->refcount == 0) {
          // Release all elements first
          array_data_t* data = (array_data_t*)cell->payload.ptr;
          for (size_t i = 0; i < data->length; i++) {
            metal_release(&data->elements[i]);
          }
          metal_free(cell->payload.ptr);
          cell->payload.ptr = NULL;
        }
      }
      break;
    case CELL_POINTER:
      // Pointers don't own the pointed-to memory
      break;
    default:
      break;
  }
}

// Cell creation functions
cell_t new_int32(int32_t value) {
  cell_t cell = {0};
  cell.type = CELL_INT32;
  cell.payload.i32 = value;
  return cell;
}

cell_t new_int64(int64_t value) {
  cell_t cell = {0};
  cell.type = CELL_INT64;
  cell.payload.i64 = value;
  return cell;
}

cell_t new_float(double value) {
  cell_t cell = {0};
  cell.type = CELL_FLOAT;
  cell.payload.f = value;
  return cell;
}

cell_t new_string(const char* utf8) {
  cell_t cell = {0};
  cell.type = CELL_STRING;

  size_t len = strlen(utf8);

  // For now, just allocate all strings. Later we'll optimize for short ones
  char* allocated = metal_alloc(len + 1);
  strcpy(allocated, utf8);
  cell.payload.ptr = allocated;

  return cell;
}

cell_t new_empty(void) {
  cell_t cell = {0};
  cell.type = CELL_EMPTY;
  return cell;
}

cell_t new_nil(void) {
  cell_t cell = {0};
  cell.type = CELL_NIL;
  return cell;
}

// Context management
void metal_init_context(context_t* ctx) {
  memset(ctx, 0, sizeof(context_t));
  ctx->name = "main";
}

// Error handling
void error(const char* msg) {
  printf("ERROR: %s\n", msg);
  // For now, just continue. Later we'll use longjmp
}

static void native_dot_s(context_t* ctx) { print_data_stack(ctx); }

static void native_bye([[maybe_unused]] context_t* ctx) {
  printf("Goodbye!\n");
  exit(0);
}

// New cell creation function
cell_t new_pointer(cell_t* target) {
  cell_t cell = {0};
  cell.type = CELL_POINTER;
  cell.payload.pointer = target;
  return cell;
}

// WORDS - List all words in the dictionary
static void native_words([[maybe_unused]] context_t* ctx) {
  printf("Dictionary (%d words):\n", dict_size);

  int words_per_line = 8;  // Adjust for readability
  for (int i = 0; i < dict_size; i++) {
    printf("%-12s", dictionary[i].name);  // Left-aligned, 12 chars wide

    if ((i + 1) % words_per_line == 0 || i == dict_size - 1) {
      printf("\n");
    }
  }
}

// Dictionary management
void add_native_word(const char* name, native_func_t func, const char* help) {
  if (dict_size >= MAX_DICT_ENTRIES) {
    error("Dictionary full");
    return;
  }

  strncpy(dictionary[dict_size].name, name, 31);
  dictionary[dict_size].name[31] = '\0';

  cell_t def = {0};
  def.type = CELL_NATIVE;
  def.payload.native = func;

  dictionary[dict_size].definition = def;
  dictionary[dict_size].help = help;  // Store help text
  dict_size++;
}

// HELP - Show help for a word
static void native_help([[maybe_unused]] context_t* ctx) {
  // We need to get the next token from input
  // For now, let's implement a simple version that shows all help
  printf("Available words with help:\n\n");

  for (int i = 0; i < dict_size; i++) {
    if (dictionary[i].help) {
      printf("%-12s %s\n", dictionary[i].name, dictionary[i].help);
    }
  }
}

int stricmp(const char* s1, const char* s2) {
  while (*s1 && *s2) {
    int c1 = tolower((unsigned char)*s1);
    int c2 = tolower((unsigned char)*s2);

    if (c1 != c2) return c1 - c2;
    s1++;
    s2++;
  }

  return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

dict_entry_t* find_word(const char* name) {
  for (int i = dict_size - 1; i >= 0; i--) {
    if (stricmp(dictionary[i].name, name) == 0) {
      return &dictionary[i];
    }
  }
  return NULL;
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
metal_result_t metal_interpret(const char* input) {
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
    dict_entry_t* word = find_word(token);
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
void init_dictionary(void) {
  add_core_words();

  add_native_word(".S", native_dot_s, "( -- ) Show stack contents");
  add_native_word("BYE", native_bye, "( -- ) Exit Metal");
  // Dictionary introspection
  add_native_word("WORDS", native_words, "( -- ) List all available words");
  add_native_word("HELP", native_help, "( -- ) Show help for all words");

  // Debug words (only when debug support compiled in)
#ifdef DEBUG_ENABLED
  add_debug_words();
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
  metal_init_context(&main_context);
  init_dictionary();
  char input[256];
  while (1) {
    printf("\nok> ");
    fflush(stdout);

    if (!platform_get_line(input, sizeof(input))) {
      break;
    }

    // Interpret the input
    metal_interpret(input);
  }

  return 0;
}