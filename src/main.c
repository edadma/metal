#include <ctype.h>
#include <setjmp.h>
#include <stdbool.h>
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

// Include our header
#include "metal.h"

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
#ifdef METAL_TARGET_PICO
  mutex_enter_blocking(&memory_mutex);
#else
  pthread_mutex_lock(&memory_mutex);
#endif

  alloc_header_t* header = malloc(sizeof(alloc_header_t) + size);
  if (!header) {
    metal_error("Out of memory");
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
      metal_error("Out of memory");
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
    metal_error("Out of memory");
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

// Stack operations
// Stack operations
void metal_push(context_t* ctx, cell_t cell) {
  if (ctx->data_stack_ptr >= DATA_STACK_SIZE) {
    metal_error("Data stack overflow");
    return;
  }

  // Retain reference if needed
  metal_retain(&cell);

  ctx->data_stack[ctx->data_stack_ptr++] = cell;
}

cell_t metal_pop(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    metal_error("Data stack underflow");
    return new_empty();
  }

  cell_t cell = ctx->data_stack[--ctx->data_stack_ptr];
  // Note: caller is responsible for the reference now
  return cell;
}

cell_t metal_peek(context_t* ctx, int depth) {
  if (depth >= ctx->data_stack_ptr || depth < 0) {
    metal_error("Stack index out of range");
    return new_empty();
  }

  return ctx->data_stack[ctx->data_stack_ptr - 1 - depth];
}

bool metal_stack_empty(context_t* ctx) { return ctx->data_stack_ptr == 0; }

// Context management
void metal_init_context(context_t* ctx) {
  memset(ctx, 0, sizeof(context_t));
  ctx->name = "main";
}

// Error handling
void metal_error(const char* msg) {
  printf("ERROR: %s\n", msg);
  // For now, just continue. Later we'll use longjmp
}

// Native word implementations
static void native_dup(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    metal_error("DUP: stack underflow");
    return;
  }

  cell_t top = metal_peek(ctx, 0);
  metal_push(ctx, top);
}

static void native_drop(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    metal_error("DROP: stack underflow");
    return;
  }

  cell_t cell = metal_pop(ctx);
  metal_release(&cell);
}

static void native_swap(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    metal_error("SWAP: insufficient stack");
    return;
  }

  cell_t a = metal_pop(ctx);
  cell_t b = metal_pop(ctx);
  metal_push(ctx, a);
  metal_push(ctx, b);
}

static void native_add(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    metal_error("+ : insufficient stack");
    return;
  }

  cell_t b = metal_pop(ctx);
  cell_t a = metal_pop(ctx);

  // Simple integer addition for now
  if (a.type == CELL_INT32 && b.type == CELL_INT32) {
    metal_push(ctx, new_int32(a.payload.i32 + b.payload.i32));
  } else {
    metal_error("+ : type mismatch");
    metal_push(ctx, a);
    metal_push(ctx, b);
  }

  metal_release(&a);
  metal_release(&b);
}

void print_cell(const cell_t* cell) {
  if (!cell) {
    printf("<null>");
    return;
  }

  switch (cell->type) {
    case CELL_INT32:
      printf("%d", cell->payload.i32);
      break;
    case CELL_INT64:
      printf("%lld", (long long)cell->payload.i64);
      break;
    case CELL_FLOAT:
      printf("%g", cell->payload.f);
      break;
    case CELL_STRING:
      printf("\"%s\"", (char*)cell->payload.ptr);
      break;
    case CELL_NIL:
      printf("[]");
      break;
    case CELL_ARRAY: {
      array_data_t* data = (array_data_t*)cell->payload.ptr;
      printf("[");

      for (size_t i = 0; i < data->length; i++) {
        if (i > 0) printf(", ");
        print_cell(&data->elements[i]);
      }

      printf("]");
      break;
    }
    case CELL_POINTER:
      printf("<pointer:%p>", (void*)cell->payload.pointer);
      break;
    case CELL_EMPTY:
      printf("<empty>");
      break;
    default:
      printf("<type %d>", cell->type);
      break;
  }
}

// Updated PRINT function to handle arrays
static void native_print(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    metal_error("PRINT: stack underflow");
    return;
  }

  cell_t cell = metal_pop(ctx);
  print_cell(&cell);
  metal_release(&cell);
}

static void native_dot_s([[maybe_unused]] context_t* ctx) {
  printf("Stack (%d): ", ctx->data_stack_ptr);
  for (int i = 0; i < ctx->data_stack_ptr; i++) {
    if (i > 0) printf(" ");
    print_cell(&ctx->data_stack[i]);
  }
  printf("\n");
}

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

// Array helper functions
array_data_t* create_array_data(size_t initial_capacity) {
  if (initial_capacity == 0) initial_capacity = 1;

  size_t alloc_size =
      sizeof(array_data_t) + (initial_capacity * sizeof(cell_t));
  array_data_t* data = metal_alloc(alloc_size);
  if (!data) return NULL;

  data->length = 0;
  data->capacity = initial_capacity;
  return data;
}

array_data_t* resize_array_data(array_data_t* data, size_t new_capacity) {
  size_t alloc_size = sizeof(array_data_t) + (new_capacity * sizeof(cell_t));
  array_data_t* new_data = metal_realloc(data, alloc_size);
  if (!new_data) return NULL;

  new_data->capacity = new_capacity;
  return new_data;
}

// Native word implementations

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

// [] - Create empty array (push CELL_NIL)
static void native_array_start(context_t* ctx) { metal_push(ctx, new_nil()); }

// , - Append element to array
static void native_comma(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    metal_error(", : insufficient stack (need array and element)");
    return;
  }

  cell_t element = metal_pop(ctx);
  cell_t array_cell = metal_pop(ctx);

  if (array_cell.type == CELL_NIL) {
    // Convert NIL to ARRAY with first element
    array_data_t* data = create_array_data(1);
    if (!data) {
      metal_push(ctx, array_cell);
      metal_push(ctx, element);
      return;
    }

    // Add the element
    data->elements[0] = element;
    data->length = 1;
    metal_retain(&element);  // Array now owns this reference

    // Create new array cell
    cell_t new_array = {0};
    new_array.type = CELL_ARRAY;
    new_array.payload.ptr = data;

    metal_push(ctx, new_array);
    metal_release(&array_cell);

  } else if (array_cell.type == CELL_ARRAY) {
    array_data_t* data = (array_data_t*)array_cell.payload.ptr;

    // Check if we need to resize
    if (data->length >= data->capacity) {
      data = resize_array_data(data, data->capacity + 1);
      if (!data) {
        metal_push(ctx, array_cell);
        metal_push(ctx, element);
        return;
      }
      // Update the array cell's pointer (realloc might have moved it)
      array_cell.payload.ptr = data;
    }

    // Add the element
    data->elements[data->length] = element;
    data->length++;
    metal_retain(&element);  // Array now owns this reference

    metal_push(ctx, array_cell);

  } else {
    metal_error(", : can only append to arrays");
    metal_push(ctx, array_cell);
    metal_push(ctx, element);
    return;
  }

  metal_release(&element);  // We retained it above, so release our reference
}

// LENGTH - Get array length
static void native_length(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    metal_error("LENGTH: stack underflow");
    return;
  }

  cell_t array_cell = metal_pop(ctx);

  if (array_cell.type == CELL_NIL) {
    metal_push(ctx, new_int32(0));
  } else if (array_cell.type == CELL_ARRAY) {
    array_data_t* data = (array_data_t*)array_cell.payload.ptr;
    metal_push(ctx, new_int32((int32_t)data->length));
  } else {
    metal_error("LENGTH: not an array");
    metal_push(ctx, array_cell);
    return;
  }

  metal_release(&array_cell);
}

// INDEX - Get pointer to array element
static void native_index(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    metal_error("INDEX: insufficient stack (need array and index)");
    return;
  }

  cell_t index_cell = metal_pop(ctx);
  cell_t array_cell = metal_pop(ctx);

  if (index_cell.type != CELL_INT32) {
    metal_error("INDEX: index must be integer");
    metal_push(ctx, array_cell);
    metal_push(ctx, index_cell);
    return;
  }

  int32_t index = index_cell.payload.i32;

  if (array_cell.type == CELL_NIL) {
    metal_error("INDEX: cannot index empty array");
    metal_push(ctx, array_cell);
    metal_push(ctx, index_cell);
    return;
  } else if (array_cell.type != CELL_ARRAY) {
    metal_error("INDEX: not an array");
    metal_push(ctx, array_cell);
    metal_push(ctx, index_cell);
    return;
  }

  array_data_t* data = (array_data_t*)array_cell.payload.ptr;

  if (index < 0 || index >= (int32_t)data->length) {
    metal_error("INDEX: index out of bounds");
    metal_push(ctx, array_cell);
    metal_push(ctx, index_cell);
    return;
  }

  // Create pointer to the element
  cell_t pointer = new_pointer(&data->elements[index]);
  metal_push(ctx, pointer);

  metal_release(&array_cell);
  metal_release(&index_cell);
}

// @ - Dereference pointer
static void native_fetch(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    metal_error("@ : stack underflow");
    return;
  }

  cell_t pointer_cell = metal_pop(ctx);

  if (pointer_cell.type != CELL_POINTER) {
    metal_error("@ : not a pointer");
    metal_push(ctx, pointer_cell);
    return;
  }

  if (!pointer_cell.payload.pointer) {
    metal_error("@ : null pointer");
    metal_push(ctx, pointer_cell);
    return;
  }

  // Push a copy of the pointed-to cell
  cell_t value = *pointer_cell.payload.pointer;
  metal_retain(&value);  // We're making a copy, so retain it
  metal_push(ctx, value);

  metal_release(&pointer_cell);
}

// ! - Store value at pointer
static void native_store(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    metal_error("! : insufficient stack (need pointer and value)");
    return;
  }

  cell_t value = metal_pop(ctx);
  cell_t pointer_cell = metal_pop(ctx);

  if (pointer_cell.type != CELL_POINTER) {
    metal_error("! : not a pointer");
    metal_push(ctx, pointer_cell);
    metal_push(ctx, value);
    return;
  }

  if (!pointer_cell.payload.pointer) {
    metal_error("! : null pointer");
    metal_push(ctx, pointer_cell);
    metal_push(ctx, value);
    return;
  }

  // Release the old value and store the new one
  metal_release(pointer_cell.payload.pointer);
  *pointer_cell.payload.pointer = value;
  metal_retain(&value);  // The pointed-to location now owns this reference

  metal_release(&pointer_cell);
  metal_release(&value);  // We retained it above
}

// Dictionary management
void add_native_word(const char* name, native_func_t func, const char* help) {
  if (dict_size >= MAX_DICT_ENTRIES) {
    metal_error("Dictionary full");
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
      metal_push(&main_context, num);
      continue;
    }

    // Try to find in dictionary
    dict_entry_t* word = find_word(token);
    if (word) {
      if (word->definition.type == CELL_NATIVE) {
        word->definition.payload.native(&main_context);
      } else {
        metal_error("Non-native words not implemented yet");
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
  add_native_word("DUP", native_dup, "( a -- a a ) Duplicate top of stack");
  add_native_word("DROP", native_drop, "( a -- ) Remove top of stack");
  add_native_word("SWAP", native_swap,
                  "( a b -- b a ) Swap top two stack items");
  add_native_word("+", native_add, "( a b -- c ) Add two numbers");
  add_native_word("PRINT", native_print, "( a -- ) Print value to output");
  add_native_word(".S", native_dot_s, "( -- ) Show stack contents");
  add_native_word("BYE", native_bye, "( -- ) Exit Metal");
  // Array words
  add_native_word("[]", native_array_start, "( -- array ) Create empty array");
  add_native_word(",", native_comma,
                  "( array item -- array ) Append item to array");
  add_native_word("LENGTH", native_length, "( array -- n ) Get array length");
  add_native_word("INDEX", native_index,
                  "( array n -- ptr ) Get pointer to array element");
  add_native_word("@", native_fetch,
                  "( ptr -- value ) Fetch value from pointer");
  add_native_word("!", native_store, "( ptr value -- ) Store value at pointer");

  // Dictionary introspection
  add_native_word("WORDS", native_words, "( -- ) List all available words");
  add_native_word("HELP", native_help, "( -- ) Show help for all words");
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
    printf("\nmetal> ");
    fflush(stdout);

    if (!platform_get_line(input, sizeof(input))) {
      break;
    }

    // Interpret the input
    metal_interpret(input);
  }

  return 0;
}