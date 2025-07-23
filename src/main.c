#include <ctype.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include our header
#include "metal.h"

// Global state (will be moved to proper context later)
static context_t main_context;
static context_t* current_ctx = &main_context;

// Simple dictionary for now - just a linear array
#define MAX_DICT_ENTRIES 256
static dict_entry_t dictionary[MAX_DICT_ENTRIES];
static int dict_size = 0;

// Memory management (basic for now)
void* metal_alloc(size_t size) {
  alloc_header_t* header = malloc(sizeof(alloc_header_t) + size);
  if (!header) {
    metal_error("Out of memory");
    return NULL;
  }
  header->refcount = 1;
  return (char*)header + sizeof(alloc_header_t);
}

void metal_free(void* ptr) {
  if (!ptr) return;
  alloc_header_t* header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  free(header);
}

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
    default:
      break;
  }
}

void metal_release(cell_t* cell) {
  if (!cell || !cell->payload.ptr) return;

  switch (cell->type) {
    case CELL_STRING:
    case CELL_OBJECT:
    case CELL_ARRAY:
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
void metal_push(cell_t cell) {
  if (current_ctx->data_stack_ptr >= DATA_STACK_SIZE) {
    metal_error("Data stack overflow");
    return;
  }

  // Retain reference if needed
  metal_retain(&cell);

  current_ctx->data_stack[current_ctx->data_stack_ptr++] = cell;
}

cell_t metal_pop(void) {
  if (current_ctx->data_stack_ptr <= 0) {
    metal_error("Data stack underflow");
    return new_empty();
  }

  cell_t cell = current_ctx->data_stack[--current_ctx->data_stack_ptr];
  // Note: caller is responsible for the reference now
  return cell;
}

cell_t metal_peek(int depth) {
  if (depth >= current_ctx->data_stack_ptr || depth < 0) {
    metal_error("Stack index out of range");
    return new_empty();
  }

  return current_ctx->data_stack[current_ctx->data_stack_ptr - 1 - depth];
}

bool metal_stack_empty(void) { return current_ctx->data_stack_ptr == 0; }

// Context management
void metal_init_context(context_t* ctx) {
  memset(ctx, 0, sizeof(context_t));
  ctx->name = "main";
}

context_t* metal_current_context(void) { return current_ctx; }

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

  cell_t top = metal_peek(0);
  metal_push(top);
}

static void native_drop(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    metal_error("DROP: stack underflow");
    return;
  }

  cell_t cell = metal_pop();
  metal_release(&cell);
}

static void native_swap(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    metal_error("SWAP: insufficient stack");
    return;
  }

  cell_t a = metal_pop();
  cell_t b = metal_pop();
  metal_push(a);
  metal_push(b);
}

static void native_add(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    metal_error("+ : insufficient stack");
    return;
  }

  cell_t b = metal_pop();
  cell_t a = metal_pop();

  // Simple integer addition for now
  if (a.type == CELL_INT32 && b.type == CELL_INT32) {
    metal_push(new_int32(a.payload.i32 + b.payload.i32));
  } else {
    metal_error("+ : type mismatch");
    metal_push(a);
    metal_push(b);
  }

  metal_release(&a);
  metal_release(&b);
}

static void native_print(context_t* ctx) {
  if (ctx->data_stack_ptr <= 0) {
    metal_error("PRINT: stack underflow");
    return;
  }

  cell_t cell = metal_pop();

  switch (cell.type) {
    case CELL_INT32:
      printf("%d", cell.payload.i32);
      break;
    case CELL_INT64:
      printf("%lld", (long long)cell.payload.i64);
      break;
    case CELL_FLOAT:
      printf("%g", cell.payload.f);
      break;
    case CELL_STRING:
      printf("%s", (char*)cell.payload.ptr);
      break;
    default:
      printf("<type %d>", cell.type);
      break;
  }

  metal_release(&cell);
}

static void native_dot_s([[maybe_unused]] context_t* ctx) {
  printf("Stack (%d): ", current_ctx->data_stack_ptr);
  for (int i = 0; i < current_ctx->data_stack_ptr; i++) {
    cell_t* cell = &current_ctx->data_stack[i];
    switch (cell->type) {
      case CELL_INT32:
        printf("%d ", cell->payload.i32);
        break;
      case CELL_INT64:
        printf("%lld ", (long long)cell->payload.i64);
        break;
      case CELL_FLOAT:
        printf("%g ", cell->payload.f);
        break;
      case CELL_STRING:
        printf("\"%s\" ", (char*)cell->payload.ptr);
        break;
      default:
        printf("<%d> ", cell->type);
        break;
    }
  }
  printf("\n");
}

static void native_bye([[maybe_unused]] context_t* ctx) {
  printf("Goodbye!\n");
  exit(0);
}

// Dictionary management
void add_native_word(const char* name, native_func_t func) {
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
  dict_size++;
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
      metal_push(num);
      continue;
    }

    // Try to find in dictionary
    dict_entry_t* word = find_word(token);
    if (word) {
      if (word->definition.type == CELL_NATIVE) {
        word->definition.payload.native(current_ctx);
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
  add_native_word("DUP", native_dup);
  add_native_word("DROP", native_drop);
  add_native_word("SWAP", native_swap);
  add_native_word("+", native_add);
  add_native_word("PRINT", native_print);
  add_native_word(".S", native_dot_s);
  add_native_word("BYE", native_bye);
}

int main(void) {
  printf("Metal Language v0.0.1\n");
  printf("Type 'bye' to exit, '.s' to show stack\n\n");

  // Initialize system
  metal_init_context(&main_context);
  init_dictionary();

  char input[256];
  while (1) {
    printf("\nmetal> ");
    fflush(stdout);

    if (!fgets(input, sizeof(input), stdin)) {
      break;
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    // Interpret the input
    metal_interpret(input);
  }

  return 0;
}