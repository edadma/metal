#include "core.h"

#include <string.h>

#include "array.h"
#include "context.h"
#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "interpreter.h"
#include "memory.h"
#include "parser.h"
#include "stack.h"
#include "util.h"

// I/O words

static void native_print(context_t* ctx) {
  require(ctx, 1, "PRINT");

  cell_t* cell = data_pop(ctx);
  print_cell(cell);
  release(cell);
}

static void native_fetch(context_t* ctx) {
  if (ctx->data_stack_ptr < 1) {
    error(ctx, "@ : stack underflow");
    return;
  }

  cell_t pointer_cell = data_pop_cell(ctx);

  if (pointer_cell.type != CELL_POINTER) {
    error(ctx, "@ : not a pointer");
  }

  if (!pointer_cell.payload.cell_ptr) {
    error(ctx, "@ : null pointer");
  }

  // Push a copy of the pointed-to cell
  cell_t value = *pointer_cell.payload.cell_ptr;
  retain(&value);  // We're making a copy, so retain it
  data_push(ctx, value);

  release(&pointer_cell);
}

static void native_store(context_t* ctx) {
  if (ctx->data_stack_ptr < 2) {
    error(ctx, "! : insufficient stack (need pointer and value)");
    return;
  }

  cell_t value = data_pop_cell(ctx);
  cell_t pointer_cell = data_pop_cell(ctx);

  // Check that value is not undefined
  if (value.type == CELL_UNDEFINED) {
    error(ctx, "! : cannot store undefined value");
  }

  if (pointer_cell.type != CELL_POINTER) {
    error(ctx, "! : not a pointer");
  }

  if (!pointer_cell.payload.cell_ptr) {
    error(ctx, "! : null pointer");
  }

  // Release the old value and store the new one
  release(pointer_cell.payload.cell_ptr);
  *pointer_cell.payload.cell_ptr = value;
  retain(&value);  // The pointed-to location now owns this reference

  release(&pointer_cell);
  release(&value);  // We retained it above
}

// Comment word
static void native_paren_comment(context_t* ctx) {
  char* comment = parse_until_char(ctx, ')');
  if (!comment) {
    error(ctx, "( : missing closing )");
  }
  // It's a comment, so just discard it
  metal_free(comment);
}

// CONSTANT ( value -- ) <name> Define a named constant
static void native_constant(context_t* ctx) {
  require(ctx, 1, "CONSTANT");
  // Parse next word as the constant name
  char word_buffer[32];
  token_type_t token_type =
      parse_next_token(&ctx->input_pos, word_buffer, sizeof(word_buffer));
  if (token_type != TOKEN_WORD) {
    error(ctx, "CONSTANT: expected constant name");
  }

  // Pop the value from stack
  cell_t value = data_pop_cell(ctx);
  // Check that value is not undefined
  if (value.type == CELL_UNDEFINED) {
    error(ctx, "CONSTANT: cannot create constant with undefined value");
  }

  // Add to dictionary
  add_cell(word_buffer, value, "User-defined constant");
  debug("Created constant '%s'", word_buffer);
}

// VARIABLE ( -- ) <name> Define a variable
static void native_variable(context_t* ctx) {
  // Parse next word as the variable name
  char word_buffer[32];
  token_type_t token_type =
      parse_next_token(&ctx->input_pos, word_buffer, sizeof(word_buffer));
  if (token_type != TOKEN_WORD) {
    error(ctx, "VARIABLE: expected variable name");
  }

  // Allocate storage for one cell, initialized to undefined
  cell_t* storage = metal_alloc(ctx, sizeof(cell_t));
  if (!storage) {
    error(ctx, "VARIABLE: allocation failed");
  }
  *storage = new_undefined();
  // Create pointer cell
  cell_t pointer_cell = new_pointer(storage);

  // Add to dictionary
  add_cell(word_buffer, pointer_cell, "User-defined variable");

  debug("Created variable '%s'", word_buffer);
}

static void native_null(context_t* ctx) { data_push(ctx, new_null()); }

static void native_undefined_check(context_t* ctx) {
  require(ctx, 1, "UNDEFINED?");

  cell_t* item = data_pop(ctx);
  bool is_undefined = item->type == CELL_UNDEFINED;

  data_push(ctx, new_boolean(is_undefined));
}

// Control flow compilation words

// IF ( condition -- ) Compile conditional branch
static void native_if(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "IF: only valid during compilation");
  }
  // Compile BRANCH_IF_FALSE with placeholder offset
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH_IF_FALSE;
  branch_cell.payload.i32 = 0;  // Placeholder for back-patching

  int branch_location = compiling_definition->length;

  compile_cell(ctx, branch_cell);
  // Push location onto return stack for back-patching
  return_push(ctx, new_int32(branch_location));
}

// ELSE ( -- ) Compile unconditional branch and back-patch IF
static void native_else(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "ELSE: only valid during compilation");
  }
  if (is_return_empty(ctx)) {
    error(ctx, "ELSE: no matching IF");
  }
  // Compile unconditional BRANCH with placeholder
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = 0;  // Placeholder for back-patching
  int else_location = compiling_definition->length;
  compile_cell(ctx, branch_cell);
  // Back-patch the IF's branch to jump HERE (to the ELSE clause that follows)
  cell_t* if_cell = return_pop(ctx);
  int if_location = if_cell->payload.i32;
  int offset = compiling_definition->length - (if_location + 1);
  compiling_definition->elements[if_location].payload.i32 = offset;
  // Push ELSE location for THEN to patch later
  return_push(ctx, new_int32(else_location));
}

// THEN ( -- ) Back-patch pending branch to jump here
static void native_then(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "THEN: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "THEN: no matching IF or ELSE");
  }

  // Back-patch the pending branch
  cell_t* branch_cell = return_pop(ctx);
  int branch_location = branch_cell->payload.i32;
  int offset = compiling_definition->length - (branch_location + 1);
  compiling_definition->elements[branch_location].payload.i32 = offset;
  release(branch_cell);
}

// BEGIN ( -- ) Mark start of loop
static void native_begin(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "BEGIN: only valid during compilation");
  }
  // Push current location for AGAIN to reference
  return_push(ctx, new_int32(compiling_definition->length));
}

// AGAIN ( -- ) Branch back to matching BEGIN
static void native_again(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "AGAIN: only valid during compilation");
  }
  if (is_return_empty(ctx)) {
    error(ctx, "AGAIN: no matching BEGIN");
  }
  // Get the BEGIN location
  cell_t* begin_cell = return_pop(ctx);
  int begin_location = begin_cell->payload.i32;
  // Calculate offset for unconditional branch back to BEGIN
  int branch_location = compiling_definition->length;
  int offset = begin_location - (branch_location + 1);

  // Compile the branch
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = offset;

  compile_cell(ctx, branch_cell);
  release(begin_cell);
}

// UNTIL ( flag -- ) Branch back to BEGIN if flag is false
static void native_until(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "UNTIL: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "UNTIL: no matching BEGIN");
  }

  // Get the BEGIN location
  cell_t* begin_cell = return_pop(ctx);
  int begin_location = begin_cell->payload.i32;

  // Calculate offset for conditional branch back to BEGIN
  int until_location = compiling_definition->length;
  int offset = begin_location - (until_location + 1);

  // Compile the conditional branch (continues loop if flag is false)
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH_IF_FALSE;
  branch_cell.payload.i32 = offset;

  compile_cell(ctx, branch_cell);
  release(begin_cell);
}

// WHILE ( flag -- ) Continue loop if flag is true, exit if false
static void native_while(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "WHILE: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "WHILE: no matching BEGIN");
  }

  // Compile conditional branch with placeholder (jumps out if false)
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH_IF_FALSE;
  branch_cell.payload.i32 = 0;  // Placeholder for REPEAT to patch

  int while_location = compiling_definition->length;

  compile_cell(ctx, branch_cell);

  // Push WHILE location for REPEAT to patch later
  return_push(ctx, new_int32(while_location));
}

// REPEAT ( -- ) Jump back to BEGIN and patch WHILE's forward jump
static void native_repeat(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "REPEAT: only valid during compilation");
  }

  if (ctx->return_stack_ptr < 2) {
    error(ctx, "REPEAT: no matching BEGIN/WHILE");
  }

  // Pop WHILE location and patch its forward jump to here
  cell_t* while_cell = return_pop(ctx);
  int while_location = while_cell->payload.i32;
  int forward_offset = compiling_definition->length + 1 - (while_location + 1);

  compiling_definition->elements[while_location].payload.i32 = forward_offset;
  // Pop BEGIN location and compile backward jump
  cell_t* begin_cell = return_pop(ctx);
  int begin_location = begin_cell->payload.i32;
  int backward_offset = begin_location - (compiling_definition->length + 1);

  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = backward_offset;
  compile_cell(ctx, branch_cell);

  release(while_cell);
  release(begin_cell);
}

// (DO) ( limit start -- ) Runtime: setup loop parameters
static void native_do_runtime(context_t* ctx) {
  require(ctx, 2, "(DO)");

  cell_t start = data_pop_cell(ctx);
  cell_t limit = data_pop_cell(ctx);

  if (start.type != CELL_INT32 || limit.type != CELL_INT32) {
    error(ctx, "(DO): loop parameters must be integers");
  }

  // Push limit then index to return stack
  return_push(ctx, limit);
  return_push(ctx, start);

  debug("DO: limit=%d start=%d", limit.payload.i32, start.payload.i32);
}

// (LOOP) ( -- ) Runtime: increment by 1 and test boundary
static void native_loop_runtime(context_t* ctx) {
  if (ctx->return_stack_ptr < 2) {
    error(ctx, "(LOOP): return stack underflow");
  }

  // Get current index and limit
  cell_t* index_cell = &ctx->return_stack[ctx->return_stack_ptr - 1];
  cell_t* limit_cell = &ctx->return_stack[ctx->return_stack_ptr - 2];

  int32_t old_index = index_cell->payload.i32;
  int32_t limit = limit_cell->payload.i32;
  int32_t new_index = old_index + 1;

  debug("LOOP: old_index=%d new_index=%d limit=%d", old_index, new_index,
        limit);

  if (new_index >= limit) {
    // Exit loop - clean up return stack and skip the branch
    release(&ctx->return_stack[ctx->return_stack_ptr - 1]);
    release(&ctx->return_stack[ctx->return_stack_ptr - 2]);
    ctx->return_stack_ptr -= 2;
    debug("LOOP: exiting, skipping branch");
    // Skip over the branch instruction
    ctx->ip++;
  } else {
    // Continue loop - update index and execute the branch
    index_cell->payload.i32 = new_index;
    debug("LOOP: continuing, executing branch");
    // Let the branch instruction execute normally
  }
}

// (+LOOP) ( n -- ) Runtime: increment by n and test boundary crossing
static void native_plus_loop_runtime(context_t* ctx) {
  require(ctx, 1, "(+LOOP)");

  if (ctx->return_stack_ptr < 2) {
    error(ctx, "(+LOOP): return stack underflow");
  }

  cell_t increment = data_pop_cell(ctx);
  if (increment.type != CELL_INT32) {
    error(ctx, "(+LOOP): increment must be integer");
  }

  // Get current index and limit
  cell_t* index_cell = &ctx->return_stack[ctx->return_stack_ptr - 1];
  cell_t* limit_cell = &ctx->return_stack[ctx->return_stack_ptr - 2];

  int32_t old_index = index_cell->payload.i32;
  int32_t limit = limit_cell->payload.i32;
  int32_t n = increment.payload.i32;
  int32_t new_index = old_index + n;

  debug("+LOOP: old_index=%d increment=%d new_index=%d limit=%d", old_index, n,
        new_index, limit);

  // ANS Forth boundary crossing logic
  bool terminate;
  if (n >= 0) {
    terminate = (old_index < limit && new_index >= limit);
  } else {
    terminate = (old_index >= limit && new_index < limit);
  }

  if (terminate) {
    // Exit loop - clean up return stack and skip the branch
    release(&ctx->return_stack[ctx->return_stack_ptr - 1]);
    release(&ctx->return_stack[ctx->return_stack_ptr - 2]);
    ctx->return_stack_ptr -= 2;
    debug("+LOOP: exiting, skipping branch");
    // Skip over the branch instruction
    ctx->ip++;
  } else {
    // Continue loop - update index and execute the branch
    index_cell->payload.i32 = new_index;
    debug("+LOOP: continuing, executing branch");
    // Let the branch instruction execute normally
  }

  release(&increment);
}

// DO ( limit start -- ) Compile loop setup
static void native_do(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "DO: only valid during compilation");
  }
  // Compile call to (DO) runtime helper
  dictionary_entry_t* do_runtime = find_word("(DO)");
  if (!do_runtime) {
    error(ctx, "DO: (DO) runtime word not found");
  }
  compile_cell(ctx, do_runtime->definition);
  // Push current location for LOOP/+LOOP to branch back to
  return_push(ctx, new_int32(compiling_definition->length));
}

// LOOP ( -- ) Compile loop increment and branch
static void native_loop(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "LOOP: only valid during compilation");
  }
  if (is_return_empty(ctx)) {
    error(ctx, "LOOP: no matching DO");
  }

  // Compile call to (LOOP) runtime helper
  dictionary_entry_t* loop_runtime = find_word("(LOOP)");
  if (!loop_runtime) {
    error(ctx, "LOOP: (LOOP) runtime word not found");
  }
  compile_cell(ctx, loop_runtime->definition);

  // Compile branch offset back to after DO
  cell_t* do_location_cell = return_pop(ctx);
  int do_location = do_location_cell->payload.i32;
  int offset = do_location - (compiling_definition->length + 1);

  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = offset;
  compile_cell(ctx, branch_cell);

  release(do_location_cell);
}

// +LOOP ( n -- ) Compile loop increment by n and branch
static void native_plus_loop(context_t* ctx) {
  if (!compilation_mode) {
    error(ctx, "+LOOP: only valid during compilation");
  }

  if (is_return_empty(ctx)) {
    error(ctx, "+LOOP: no matching DO");
  }

  // Compile call to (+LOOP) runtime helper
  dictionary_entry_t* plus_loop_runtime = find_word("(+LOOP)");
  if (!plus_loop_runtime) {
    error(ctx, "+LOOP: (+LOOP) runtime word not found");
  }
  compile_cell(ctx, plus_loop_runtime->definition);

  // Compile branch offset back to after DO
  cell_t* do_location_cell = return_pop(ctx);
  int do_location = do_location_cell->payload.i32;
  int offset = do_location - (compiling_definition->length + 1);
  cell_t branch_cell = {0};
  branch_cell.type = CELL_BRANCH;
  branch_cell.payload.i32 = offset;
  compile_cell(ctx, branch_cell);

  release(do_location_cell);
}

// I ( -- index ) Get current loop index
static void native_i(context_t* ctx) {
  if (ctx->return_stack_ptr < 1) {
    error(ctx, "I: no active loop");
  }
  // Index is on top of return stack
  cell_t index = ctx->return_stack[ctx->return_stack_ptr - 1];
  data_push(ctx, index);
}

// J ( -- outer_index ) Get outer loop index
static void native_j(context_t* ctx) {
  if (ctx->return_stack_ptr < 3) {
    error(ctx, "J: no nested loop");
  }

  // Outer index is at depth 2: [..., outer_limit, outer_index, inner_limit,
  // inner_index]
  cell_t outer_index = ctx->return_stack[ctx->return_stack_ptr - 3];
  data_push(ctx, outer_index);
}

// UNLOOP ( -- ) Remove loop parameters from return stack
static void native_unloop(context_t* ctx) {
  if (ctx->return_stack_ptr < 2) {
    error(ctx, "UNLOOP: no active loop");
  }

  // Remove limit and index from return stack
  release(&ctx->return_stack[ctx->return_stack_ptr - 1]);
  release(&ctx->return_stack[ctx->return_stack_ptr - 2]);
  ctx->return_stack_ptr -= 2;
}

// ' (tick) - Get code cell from dictionary
static void native_tick(context_t* ctx) {
  char word_buffer[32];
  token_type_t token_type =
      parse_next_token(&ctx->input_pos, word_buffer, sizeof(word_buffer));

  if (token_type != TOKEN_WORD) {
    error(ctx, "' : expected word name");
  }

  dictionary_entry_t* entry = find_word(word_buffer);
  if (!entry) {
    error(ctx, "' : word '%s' not found", word_buffer);
  }

  // Push the definition cell
  data_push(ctx, entry->definition);
  debug("' pushed definition for '%s'", word_buffer);
}

// ['] (bracket tick) - Compile-time get code cell from dictionary
static void native_bracket_tick(context_t* ctx) {
  if (!compilation_mode && !ctx->anonymous_compilation_mode) {
    error(ctx, "['] : only valid during compilation");
  }

  char word_buffer[32];
  token_type_t token_type =
      parse_next_token(&ctx->input_pos, word_buffer, sizeof(word_buffer));

  if (token_type != TOKEN_WORD) {
    error(ctx, "['] : expected word name");
  }

  dictionary_entry_t* entry = find_word(word_buffer);
  if (!entry) {
    error(ctx, "['] : word '%s' not found", word_buffer);
  }

  // Compile LITERAL followed by the code cell
  dictionary_entry_t* literal_word = find_word("LITERAL");
  if (!literal_word) {
    error(ctx, "['] : LITERAL word not found");
  }

  compile_cell(ctx, literal_word->definition);
  compile_cell(ctx, entry->definition);
  debug("['] compiled definition for '%s'", word_buffer);
}

// EXECUTE - Execute a code cell
static void native_execute(context_t* ctx) {
  require(ctx, 1, "EXECUTE");

  cell_t code_cell = data_pop_cell(ctx);

  switch (code_cell.type) {
    case CELL_NATIVE:
      debug("EXECUTE: calling native function");
      code_cell.payload.native(ctx);
      break;

    case CELL_CODE:
      debug("EXECUTE: executing code array");
      return_push(ctx, new_return(ctx->ip));
      ctx->ip = code_cell.payload.array->elements;
      execute_code(ctx);
      break;

    default:
      error(ctx, "EXECUTE: not executable (type %d)", code_cell.type);
  }

  release(&code_cell);
}

// LITERAL ( -- value ) Runtime: push next compiled cell as data
static void native_literal(context_t* ctx) {
  if (!ctx->ip) {
    error(ctx, "LITERAL: no instruction pointer");
  }

  // Push the next cell as data, don't execute it
  cell_t* literal_cell = ctx->ip;
  ctx->ip++;  // Skip over the literal data

  data_push_ptr(ctx, literal_cell);
  debug("LITERAL: pushed cell type %d as data", literal_cell->type);
}

// Memory combination words

static void native_plus_store(context_t* ctx) {
  require(ctx, 2, "+!");

  cell_t* addr_cell = data_pop(ctx);
  cell_t* value_cell = data_pop(ctx);

  if (addr_cell->type != CELL_POINTER) {
    error(ctx, "+! : second argument must be a pointer");
  }

  if (!addr_cell->payload.cell_ptr) {
    error(ctx, "+! : null pointer");
  }

  cell_t* target = addr_cell->payload.cell_ptr;

  // Efficient in-place addition based on target type
  switch (target->type) {
    case CELL_INT32:
      if (value_cell->type == CELL_INT32) {
        target->payload.i32 += value_cell->payload.i32;
      } else if (value_cell->type == CELL_INT64) {
        // Promote to INT64 if needed
        int64_t result = (int64_t)target->payload.i32 + value_cell->payload.i64;
        target->type = CELL_INT64;
        target->payload.i64 = result;
      } else if (value_cell->type == CELL_FLOAT) {
        // Promote to FLOAT
        target->type = CELL_FLOAT;
        target->payload.f64 =
            (double)target->payload.i32 + value_cell->payload.f64;
      } else {
        error(ctx, "+! : incompatible types");
      }
      break;

    case CELL_INT64:
      if (value_cell->type == CELL_INT32) {
        target->payload.i64 += (int64_t)value_cell->payload.i32;
      } else if (value_cell->type == CELL_INT64) {
        target->payload.i64 += value_cell->payload.i64;
      } else if (value_cell->type == CELL_FLOAT) {
        // Promote to FLOAT
        target->type = CELL_FLOAT;
        target->payload.f64 =
            (double)target->payload.i64 + value_cell->payload.f64;
      } else {
        error(ctx, "+! : incompatible types");
      }
      break;

    case CELL_FLOAT:
      if (value_cell->type == CELL_INT32) {
        target->payload.f64 += (double)value_cell->payload.i32;
      } else if (value_cell->type == CELL_INT64) {
        target->payload.f64 += (double)value_cell->payload.i64;
      } else if (value_cell->type == CELL_FLOAT) {
        target->payload.f64 += value_cell->payload.f64;
      } else {
        error(ctx, "+! : incompatible types");
      }
      break;

    default:
      error(ctx, "+! : target must be numeric");
  }

  release(addr_cell);
  release(value_cell);
}

static void native_one_plus_store(context_t* ctx) {
  require(ctx, 1, "1+!");

  cell_t* addr_cell = data_pop(ctx);

  if (addr_cell->type != CELL_POINTER) {
    error(ctx, "1+! : argument must be a pointer");
  }

  if (!addr_cell->payload.cell_ptr) {
    error(ctx, "1+! : null pointer");
  }

  cell_t* target = addr_cell->payload.cell_ptr;

  // Efficient in-place increment
  switch (target->type) {
    case CELL_INT32:
      target->payload.i32++;
      break;
    case CELL_INT64:
      target->payload.i64++;
      break;
    case CELL_FLOAT:
      target->payload.f64 += 1.0;
      break;
    default:
      error(ctx, "1+! : target must be numeric");
  }

  release(addr_cell);
}

static void native_one_minus_store(context_t* ctx) {
  require(ctx, 1, "1-!");

  cell_t* addr_cell = data_pop(ctx);

  if (addr_cell->type != CELL_POINTER) {
    error(ctx, "1-! : argument must be a pointer");
  }

  if (!addr_cell->payload.cell_ptr) {
    error(ctx, "1-! : null pointer");
  }

  cell_t* target = addr_cell->payload.cell_ptr;

  // Efficient in-place decrement
  switch (target->type) {
    case CELL_INT32:
      target->payload.i32--;
      break;
    case CELL_INT64:
      target->payload.i64--;
      break;
    case CELL_FLOAT:
      target->payload.f64 -= 1.0;
      break;
    default:
      error(ctx, "1-! : target must be numeric");
  }

  release(addr_cell);
}

// Internal words - not in dictionary
static const cell_t literal_cell = {.type = CELL_NATIVE,
                                    .flags = 0,
                                    .word_idx = -1,
                                    .payload.native = native_literal};

static const cell_t do_runtime_cell = {.type = CELL_NATIVE,
                                       .flags = 0,
                                       .word_idx = -1,
                                       .payload.native = native_do_runtime};

static const cell_t loop_runtime_cell = {.type = CELL_NATIVE,
                                         .flags = 0,
                                         .word_idx = -1,
                                         .payload.native = native_loop_runtime};

static const cell_t plus_loop_runtime_cell = {
    .type = CELL_NATIVE,
    .flags = 0,
    .word_idx = -1,
    .payload.native = native_plus_loop_runtime};

// Register all core words
void add_core_words(void) {
  add_native_word("NULL", native_null, "( -- null ) Push null value");
  add_native_word("UNDEFINED?", native_undefined_check,
                  "( a -- bool ) Test if value is undefined");

  // I/O
  add_native_word("PRINT", native_print, "( a -- ) Print value to output");

  add_native_word("@", native_fetch,
                  "( ptr -- value ) Fetch value from pointer");
  add_native_word("!", native_store, "( ptr value -- ) Store value at pointer");

  // Control flow (compilation only)
  add_native_word_immediate("IF", native_if, "( bool -- ) Begin conditional");
  add_native_word_immediate("ELSE", native_else, "( -- ) Alternative branch");
  add_native_word_immediate("THEN", native_then, "( -- ) End conditional");

  add_native_word("(", native_paren_comment,
                  "( comment -- ) Parenthesis comment until )");
  add_native_word_immediate("BEGIN", native_begin, "( -- ) Mark start of loop");
  add_native_word_immediate("AGAIN", native_again,
                            "( -- ) Branch back to BEGIN");
  add_native_word_immediate(
      "UNTIL", native_until,
      "( flag -- ) Branch back to BEGIN if flag is false");
  add_native_word_immediate("WHILE", native_while,
                            "( flag -- ) Continue loop if flag is true");
  add_native_word_immediate("REPEAT", native_repeat,
                            "( -- ) Jump back to BEGIN");

  // DO/LOOP constructs
  add_native_word("(DO)", native_do_runtime,
                  "( limit start -- ) Runtime: setup loop");
  add_native_word("(LOOP)", native_loop_runtime,
                  "( -- ) Runtime: increment and test");
  add_native_word("(+LOOP)", native_plus_loop_runtime,
                  "( n -- ) Runtime: increment by n");
  add_native_word_immediate("DO", native_do,
                            "( limit start -- ) Begin counted loop");
  add_native_word_immediate("LOOP", native_loop,
                            "( -- ) End loop, increment by 1");
  add_native_word_immediate("+LOOP", native_plus_loop,
                            "( n -- ) End loop, increment by n");
  add_native_word("I", native_i, "( -- index ) Current loop index");
  add_native_word("J", native_j, "( -- outer_index ) Outer loop index");
  add_native_word("UNLOOP", native_unloop, "( -- ) Remove loop parameters");

  // Memory combination words
  add_native_word("+!", native_plus_store,
                  "( n addr -- ) Add n to memory location");
  add_native_word("1+!", native_one_plus_store,
                  "( addr -- ) Increment memory location");
  add_native_word("1-!", native_one_minus_store,
                  "( addr -- ) Decrement memory location");

  add_native_word("CONSTANT", native_constant,
                  "( value -- ) <name> Define named constant");
  add_native_word("VARIABLE", native_variable, "( -- ) <name> Define variable");

  add_native_word("'", native_tick,
                  "( -- code ) <name> Get code from dictionary");
  add_native_word_immediate("[']", native_bracket_tick,
                            "( -- code ) <name> Compile code from dictionary");
  add_native_word("EXECUTE", native_execute, "( code -- ) Execute code cell");

  add_definition("MIN", "2DUP > IF SWAP THEN DROP",
                 "( a b -- min ) Return minimum of two numbers");
  add_definition("MAX", "2DUP < IF SWAP THEN DROP",
                 "( a b -- max ) Return maximum of two numbers");
  add_definition("ROT", "2 ROLL", "( a b c -- b c a ) Rotate top three items");
  add_definition("SIGNUM", "DUP 0 < IF DROP -1 ELSE 0 > IF 1 ELSE 0 THEN THEN",
                 "( n -- -1|0|1 ) Return sign of number");
  add_definition("CONST", "CONSTANT", "( value -- ) Define constant");
}
