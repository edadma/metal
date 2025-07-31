#include "interpreter.h"

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "context.h"
#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "memory.h"
#include "parser.h"
#include "stack.h"
#include "util.h"

#define MAX_TOKEN_SIZE 256

typedef union {
  char buffer[sizeof(string_t) + MAX_TOKEN_SIZE * 4];
  string_t string;
} string_buffer_t;

// Global compilation state
bool compilation_mode = false;
cell_array_t* compiling_definition = NULL;
char compiling_word_name[MAX_NAME_LENGTH];

// Helper function to add a compiled word definition from source
void add_definition(const char* name, const char* source, const char* help) {
  // Save current compilation state
  bool saved_compilation_mode = compilation_mode;
  cell_array_t* saved_compiling_definition = compiling_definition;
  char saved_compiling_word_name[MAX_NAME_LENGTH];
  strncpy(saved_compiling_word_name, compiling_word_name, sizeof(saved_compiling_word_name));
  // Set up compilation
  compilation_mode = true;
  compiling_definition = create_array_data(&main_context, 8);
  if (!compiling_definition) {
    error(&main_context, "add_definition: allocation failed for %s", name);
  }
  strncpy(compiling_word_name, name, sizeof(compiling_word_name) - 1);
  compiling_word_name[sizeof(compiling_word_name) - 1] = '\0';
  // Compile the source code
  metal_result_t result = interpret(&main_context, true, source);
  if (result != METAL_OK) {
    // Clean up on error
    if (compiling_definition) {
      for (size_t i = 0; i < compiling_definition->length; i++) {
        release(&compiling_definition->elements[i]);
      }
      metal_free(compiling_definition);
    }
    error(&main_context, "add_definition: failed to compile %s", name);
  }
  // Add EXIT to the end of the definition
  dictionary_entry_t* exit_word = find_word("EXIT");
  if (!exit_word) {
    error(&main_context, "add_definition: EXIT word not found");
  }
  if (compiling_definition->length >= compiling_definition->capacity) {
    compiling_definition = resize_array_data(&main_context, compiling_definition, compiling_definition->capacity * 2);
    if (!compiling_definition) {
      error(&main_context, "add_definition: failed to resize definition for %s", name);
    }
  }
  compiling_definition->elements[compiling_definition->length] = exit_word->definition;
  compiling_definition->length++;
  retain(&exit_word->definition);

  // Create the code cell and add to dictionary
  cell_t code_cell = new_code(compiling_definition);
  add_cell(name, code_cell, help);

  // Restore compilation state
  compilation_mode = saved_compilation_mode;
  compiling_definition = saved_compiling_definition;
  strncpy(compiling_word_name, saved_compiling_word_name, sizeof(compiling_word_name));
}

// Number parsing
// Number parsing
bool try_parse_number(context_t* ctx, const char* token, cell_t* result) {
  char* endptr;
  size_t len = strlen(token);

  // Check for explicit type suffixes
  bool force_int64 = false;
  bool force_float = false;
  char* working_token = (char*)token;
  char saved_char = '\0';  // To restore the original character

  if (len > 1) {
    char last_char = token[len - 1];
    if (last_char == 'L' || last_char == 'l') {
      force_int64 = true;
      // Temporarily null-terminate one character earlier
      saved_char = working_token[len - 1];
      working_token[len - 1] = '\0';
    } else if (last_char == 'F' || last_char == 'f') {
      force_float = true;
      // Temporarily null-terminate one character earlier
      saved_char = working_token[len - 1];
      working_token[len - 1] = '\0';
    }
  }

  // Try integer parsing first
  errno = 0;
  long long val = strtoll(working_token, &endptr, 10);

  if (*endptr == '\0') {
    // It's definitely meant to be an integer
    if (errno == ERANGE) {
      // Restore original character before error
      if (saved_char) working_token[len - 1] = saved_char;
      error(ctx, "Integer literal out of range: %s", token);
      return false;  // Never reached due to error()
    }

    if (force_float) {
      *result = new_float((double)val);
    } else if (force_int64 || val < INT32_MIN || val > INT32_MAX) {
      *result = new_int64(val);
    } else {
      *result = new_int32((int32_t)val);
    }

    // Restore original character
    if (saved_char) working_token[len - 1] = saved_char;
    return true;
  }

  // Try float parsing
  errno = 0;
  double fval = strtod(working_token, &endptr);

  if (*endptr == '\0') {
    // It's definitely meant to be a float
    if (errno == ERANGE) {
      // Restore original character before error
      if (saved_char) working_token[len - 1] = saved_char;
      error(ctx, "Float literal out of range: %s", token);
      return false;  // Never reached due to error()
    }

    *result = new_float(fval);
    // Restore original character
    if (saved_char) working_token[len - 1] = saved_char;
    return true;
  }

  // Restore original character and return false - not a number at all
  if (saved_char) working_token[len - 1] = saved_char;
  return false;
}

void execute_code(context_t* ctx) {
  while (ctx->ip) {
    cell_t* cell = ctx->ip;

    ctx->ip++;

    debug("executing cell type: %d", cell->type);
    switch (cell->type) {
      case CELL_BRANCH_IF_FALSE: {
        // Conditional branch: jump if top of stack is falsy
        if (is_data_empty(ctx)) {
          error(ctx, "BRANCH_IF_FALSE: stack underflow");
        }

        cell_t* condition = data_pop(ctx);

        if (!is_truthy(condition)) {
          // Jump: ip += offset
          ctx->ip += cell->payload.i32;
        }

        release(condition);
        break;
      }
      case CELL_BRANCH: {
        // Unconditional branch
        ctx->ip += cell->payload.i32;
        break;
      }
      case CELL_NATIVE: {
        // Execute native function
        cell->payload.native(ctx);
        break;
      }
      case CELL_CODE: {
        return_push(ctx, new_return(ctx->ip));
        ctx->ip = cell->payload.array->elements;
        break;
      }
        // All other cell types push themselves onto the stack
      default: {
        data_push(ctx, *cell);
        break;
      }
    }
  }
}

// Main interpreter
metal_result_t interpret(context_t* ctx, bool print_errors, const char* input) {
  // Set up exception handling
  if (setjmp(ctx->error_jmp) != 0) {
    // We jumped here due to an error
    if (print_errors) printf("ERROR: %s\n", ctx->error_msg);

    // Clear parsing state
    ctx->input_pos = NULL;
    ctx->input_start = NULL;

    return METAL_ERROR;
  }

  // Set up parsing state in context
  ctx->input_start = input;
  ctx->input_pos = input;

  char token_buffer[MAX_TOKEN_SIZE];
  token_type_t token_type;

  // Parse and execute tokens one at a time
  while ((token_type = parse_next_token(ctx, &ctx->input_pos, token_buffer, sizeof(token_buffer))) != TOKEN_EOF) {
    if (token_type == TOKEN_STRING) {
      // String literal

      // Convert C string to stack string_t
      string_buffer_t string_buf;
      string_t* local_str = &string_buf.string;

      string_from_cstr(token_buffer, local_str);

      // Use for either interned or allocated strings
      if (compilation_mode) {
        // Try to intern
        const string_t* existing = intern_lookup(ctx, local_str);
        if (existing) {
          compile_cell(ctx, new_interned_string(existing));
        } else {
          const string_t* newly_interned = intern_add(ctx, local_str);

          compile_cell(ctx, new_interned_string(newly_interned));
        }
      } else {
        data_push(ctx, new_allocated_string(ctx, local_str));
      }
    } else if (token_type == TOKEN_WORD) {
      char* word = token_buffer;

      // Try to parse as number first
      cell_t num;
      if (try_parse_number(ctx, word, &num)) {
        if (compilation_mode) {
          compile_cell(ctx, num);
        } else {
          data_push(ctx, num);
        }
        continue;
      }

      // Try to find in dictionary
      const dictionary_entry_t* dict_word = find_word(word);

      if (dict_word) {
        // Check if word is immediate (executes even during compilation)
        bool is_immediate = (dict_word->definition.flags & CELL_FLAG_IMMEDIATE) != 0;

        if (compilation_mode && !is_immediate) {
          // Compile the word reference
          compile_cell(ctx, dict_word->definition);
        } else {
          // Execute the word (either interpretation mode or immediate word)
          if (dict_word->definition.type == CELL_NATIVE) {
            dict_word->definition.payload.native(ctx);
          } else if (dict_word->definition.type == CELL_CODE) {
            debug("executing word: %s", dict_word->name);
            ctx->ip = dict_word->definition.payload.array->elements;
            execute_code(ctx);
          } else {
            // For constants and variables - just push the cell onto the stack
            data_push(ctx, dict_word->definition);
          }
        }
        continue;
      }

      error(ctx, "Unknown word: %s", word);
    }
  }

  // Clear parsing state on success
  ctx->input_pos = NULL;
  ctx->input_start = NULL;

  return METAL_OK;
}

void compile_cell(context_t* ctx, cell_t cell) {
  cell_array_t* target_definition;

  if (ctx->anonymous_compilation_mode) {
    target_definition = ctx->compiling_anonymous_definition;
  } else if (compilation_mode) {
    target_definition = compiling_definition;
  } else {
    error(ctx, "compile_cell: not in compilation mode");
    return;
  }

  if (target_definition->length >= target_definition->capacity) {
    target_definition = resize_array_data(ctx, target_definition, target_definition->capacity * 2);
    if (!target_definition) {
      error(ctx, "compile_cell: failed to resize definition");
      return;
    }

    // Update the global pointer
    if (ctx->anonymous_compilation_mode) {
      ctx->compiling_anonymous_definition = target_definition;
    } else {
      compiling_definition = target_definition;
    }
  }

  target_definition->elements[target_definition->length] = cell;
  target_definition->length++;
  retain(&cell);
}