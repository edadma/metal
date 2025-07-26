#include "interpreter.h"

#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "context.h"
#include "core.h"
#include "dictionary.h"
#include "memory.h"
#include "parser.h"
#include "stack.h"

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

void execute_code(context_t* ctx, cell_array_t* code) {
  for (size_t i = 0; i < code->length; i++) {
    cell_t* cell = &code->elements[i];
    switch (cell->type) {
      case CELL_NATIVE: {
        // Execute native function
        cell->payload.native(ctx);
        break;
      }

      case CELL_CODE: {
        // Recursive call to execute nested code
        execute_code(ctx, cell->payload.ptr);
        break;
      }

        // All other cell types push themselves onto the stack
      case CELL_INT32:
      case CELL_INT64:
      case CELL_FLOAT:
      case CELL_STRING:
      case CELL_ARRAY:
      case CELL_NIL:
      case CELL_EMPTY:
      default: {
        cell_t copy = *cell;
        metal_retain(&copy);
        data_push(ctx, copy);
        break;
      }
    }
  }
}

// Main interpreter
metal_result_t interpret(context_t* ctx, const char* input) {
  // Set up exception handling
  if (setjmp(ctx->error_jmp) != 0) {
    // We jumped here due to an error
    printf("ERROR: %s\n", ctx->error_msg);

    // Clear parsing state
    ctx->input_pos = NULL;
    ctx->input_start = NULL;

    return METAL_ERROR;
  }

  // Set up parsing state in context
  ctx->input_start = input;
  ctx->input_pos = input;

  char token_buffer[256];
  token_type_t token_type;

  // Parse and execute tokens one at a time
  while ((token_type = parse_next_token(&ctx->input_pos, token_buffer,
                                        sizeof(token_buffer))) != TOKEN_EOF) {
    if (token_type == TOKEN_STRING) {
      // String literal
      cell_t string_cell = new_string(token_buffer);

      if (compilation_mode) {
        compile_cell(string_cell);
      } else {
        data_push(ctx, string_cell);
      }

    } else if (token_type == TOKEN_WORD) {
      char* word = token_buffer;

      // Try to parse as number first
      cell_t num;
      if (try_parse_number(word, &num)) {
        if (compilation_mode) {
          compile_cell(num);
        } else {
          data_push(ctx, num);
        }
        continue;
      }

      // Try to find in dictionary
      const dictionary_entry_t* dict_word = find_word(word);

      if (dict_word) {
        // Check if word is immediate (executes even during compilation)
        bool is_immediate =
            (dict_word->definition.flags & CELL_FLAG_IMMEDIATE) != 0;

        if (compilation_mode && !is_immediate) {
          // Compile the word reference
          compile_cell(dict_word->definition);
        } else {
          // Execute the word (either interpretation mode or immediate word)
          if (dict_word->definition.type == CELL_NATIVE) {
            dict_word->definition.payload.native(ctx);
          } else if (dict_word->definition.type == CELL_CODE) {
            execute_code(ctx, (cell_array_t*)dict_word->definition.payload.ptr);
          } else {
            // Store error in context and longjmp
            static char error_buf[256];
            snprintf(error_buf, sizeof(error_buf), "Unknown word type: %d",
                     dict_word->definition.type);
            ctx->error_msg = error_buf;
            longjmp(ctx->error_jmp, 1);
          }
        }
        continue;
      }

      // Unknown word - store error in context and longjmp
      static char error_buf[270];
      snprintf(error_buf, sizeof(error_buf), "Unknown word: %s", word);
      ctx->error_msg = error_buf;
      longjmp(ctx->error_jmp, 1);
    }
  }

  // Clear parsing state on success
  ctx->input_pos = NULL;
  ctx->input_start = NULL;

  return METAL_OK;
}

void compile_cell(cell_t cell) {
  if (compiling_definition->length >= compiling_definition->capacity) {
    compiling_definition = resize_array_data(
        compiling_definition, compiling_definition->capacity * 2);
    if (!compiling_definition) {
      error(&main_context, "Compilation: failed to resize definition");
      return;
    }
  }

  compiling_definition->elements[compiling_definition->length] = cell;
  compiling_definition->length++;
  metal_retain(&cell);
}
