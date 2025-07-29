#include "core_primitive.h"

#include <string.h>

#include "array.h"
#include "context.h"
#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "interpreter.h"
#include "parser.h"
#include "stack.h"

static void native_exit(context_t* ctx) {
  debug("executing EXIT");
  if (is_return_empty(ctx)) {
    debug("EXIT: no return: set ip = NULL");
    ctx->ip = NULL;
  } else {
    cell_t* ret = return_pop(ctx);

    if (ret->type != CELL_RETURN)
      error(ctx, "Invalid return value");
    else
      ctx->ip = ret->payload.cell_ptr;
  }
}

static void native_def(context_t* ctx) {
  if (compilation_mode) {
    error(ctx, "DEF: already in compilation mode");
  }

  // Parse next word as the definition name
  char word_buffer[MAX_NAME_LENGTH];
  token_type_t token_type =
      parse_next_token(ctx,&ctx->input_pos, word_buffer, sizeof(word_buffer));
  if (token_type != TOKEN_WORD) {
    error(ctx, "DEF: expected word name");
  }

  // Initialize compilation
  compiling_definition =
      create_array_data(ctx, 8);  // Start with small capacity
  if (!compiling_definition) {
    error(ctx, "DEF: allocation failed");
  }

  strncpy(compiling_word_name, word_buffer, sizeof(compiling_word_name) - 1);
  compiling_word_name[sizeof(compiling_word_name) - 1] = '\0';
  compilation_mode = true;
  debug("Started compiling word '%s'", compiling_word_name);
}

// Better approach for native_end() - don't use NULL function
static void native_end(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "END: not in compilation mode");
  }

  // Add EXIT to the end of the definition
  dictionary_entry_t* exit_word = find_word("EXIT");
  if (!exit_word) {
    error(ctx, "END: EXIT word not found");
  }

  // Add EXIT as the last instruction
  if (compiling_definition->length >= compiling_definition->capacity) {
    compiling_definition = resize_array_data(
        ctx, compiling_definition, compiling_definition->capacity * 2);
    if (!compiling_definition) {
      error(ctx, "END: failed to resize definition");
    }
  }

  compiling_definition->elements[compiling_definition->length] =
      exit_word->definition;
  compiling_definition->length++;
  retain(&exit_word->definition);

  // Create the code cell
  cell_t code_cell = new_code(compiling_definition);

  add_cell(compiling_word_name, code_cell, "User-defined word");

  // Reset compilation state
  compilation_mode = false;
  compiling_definition = NULL;  // Now owned by the dictionary
  compiling_word_name[0] = '\0';

  debug("Finished compiling word '%s'", compiling_word_name);
}

// Register all core primitive words
void add_core_primitive_words(void) {
  add_native_word_immediate("DEF", native_def,
                            "( -- ) <name> Start word definition");
  add_native_word_immediate("END", native_end, "( -- ) End word definition");
  add_native_word("EXIT", native_exit, "( -- ) Exit from word definition");
}