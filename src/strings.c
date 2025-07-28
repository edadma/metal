#include "strings.h"

#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "error.h"
#include "interpreter.h"
#include "memory.h"
#include "stack.h"

// Create string from C string
cell_t string_from_cstr(context_t* ctx, const char* cstr) {
  return new_string(ctx, cstr);
}

// Get string length (handles NULL payload)
size_t string_length(cell_t* str) {
  if (str->type != CELL_STRING || !str->payload.utf8) {
    return 0;
  }
  return str->payload.utf8->length;
}

// Check if string is empty
bool string_is_empty(cell_t* str) { return string_length(str) == 0; }

// Concatenate two strings (handles empty strings with NULL payloads)
cell_t string_concat(context_t* ctx, cell_t* a, cell_t* b) {
  if (a->type != CELL_STRING || b->type != CELL_STRING) {
    error(ctx, "string_concat: both arguments must be strings");
  }

  size_t alen = string_length(a);
  size_t blen = string_length(b);

  // If both empty, return empty string
  if (alen == 0 && blen == 0) {
    return new_empty_string();
  }

  // If one is empty, return copy of the other
  if (alen == 0) {
    retain(b);
    return *b;
  }
  if (blen == 0) {
    retain(a);
    return *a;
  }

  // Both have content - allocate new string
  uint8_array_t* new_str =
      metal_alloc(ctx, sizeof(uint8_array_t) + alen + blen);
  if (!new_str) {
    error(ctx, "string_concat: allocation failed");
  }

  // Initialize the new string
  new_str->refcount = 1;
  new_str->length = alen + blen;
  new_str->capacity = alen + blen;

  // Copy both strings
  memcpy(new_str->data, a->payload.utf8->data, alen);
  memcpy(new_str->data + alen, b->payload.utf8->data, blen);

  // Create result cell
  cell_t result = {0};
  result.type = CELL_STRING;
  result.payload.utf8 = new_str;

  return result;
}

// Append C string to Metal string
cell_t string_append_cstr(context_t* ctx, cell_t* str, const char* cstr) {
  if (str->type != CELL_STRING) {
    error(ctx, "string_append_cstr: first argument must be string");
  }

  cell_t cstr_cell = string_from_cstr(ctx, cstr);
  cell_t result = string_concat(ctx, str, &cstr_cell);
  release(&cstr_cell);
  return result;
}

// Append single character to string
cell_t string_append_char(context_t* ctx, cell_t* str, char c) {
  char char_str[2] = {c, '\0'};
  return string_append_cstr(ctx, str, char_str);
}

// Convert cell to C string representation
void cell_to_cstr(cell_t* cell, char* buffer, size_t buffer_size) {
  if (!cell || !buffer || buffer_size == 0) {
    return;
  }

  switch (cell->type) {
    case CELL_INT32:
      snprintf(buffer, buffer_size, "%d", cell->payload.i32);
      break;

    case CELL_INT64:
      snprintf(buffer, buffer_size, "%lld", (long long)cell->payload.i64);
      break;

    case CELL_FLOAT:
      snprintf(buffer, buffer_size, "%g", cell->payload.f64);
      break;

    case CELL_STRING:
      if (!cell->payload.utf8) {
        buffer[0] = '\0';  // Empty string
      } else {
        size_t len = cell->payload.utf8->length;
        if (len >= buffer_size) len = buffer_size - 1;
        memcpy(buffer, cell->payload.utf8->data, len);
        buffer[len] = '\0';
      }
      break;

    case CELL_BOOLEAN:
      snprintf(buffer, buffer_size, "%s",
               cell->payload.boolean ? "true" : "false");
      break;

    case CELL_ARRAY:
      if (!cell->payload.array) {
        snprintf(buffer, buffer_size, "[]");
      } else {
        snprintf(buffer, buffer_size, "[array:%d]",
                 (int)cell->payload.array->length);
      }
      break;

    case CELL_OBJECT:
      if (!cell->payload.object) {
        snprintf(buffer, buffer_size, "{}");
      } else {
        snprintf(buffer, buffer_size, "{object:%d}",
                 (int)cell->payload.object->length);
      }
      break;

    case CELL_NULL:
      snprintf(buffer, buffer_size, "null");
      break;

    case CELL_UNDEFINED:
      snprintf(buffer, buffer_size, "undefined");
      break;

    default:
      snprintf(buffer, buffer_size, "<type %d>", cell->type);
      break;
  }
}

// String operations for Metal language

static void native_string_empty_q(context_t* ctx) {
  require(ctx, 1, "STRING-EMPTY?");
  cell_t* str = data_pop(ctx);

  if (str->type != CELL_STRING) {
    error(ctx, "STRING-EMPTY?: argument must be string");
  }

  bool empty = string_is_empty(str);
  release(str);
  data_push(ctx, new_boolean(empty));
}

static void native_string_length(context_t* ctx) {
  require(ctx, 1, "STRING-LENGTH");
  cell_t* str = data_pop(ctx);

  if (str->type != CELL_STRING) {
    error(ctx, "STRING-LENGTH: argument must be string");
  }

  size_t len = string_length(str);
  release(str);
  data_push(ctx, new_int32((int32_t)len));
}

static void native_string_concat(context_t* ctx) {
  require(ctx, 2, "STRING-CONCAT");
  cell_t* b = data_pop(ctx);
  cell_t* a = data_pop(ctx);

  cell_t result = string_concat(ctx, a, b);

  release(a);
  release(b);
  data_push(ctx, result);
}

// Register all string words
void add_string_words(void) {
  add_native_word("STRING-EMPTY?", native_string_empty_q,
                  "( string -- bool ) Test if string is empty");
  add_native_word("STRING-LENGTH", native_string_length,
                  "( string -- n ) Get string length");
  add_native_word("STRING-CONCAT", native_string_concat,
                  "( str1 str2 -- str3 ) Concatenate two strings");
  // add_native_word(
  //     "FORMAT", native_format,
  //     "( args... format -- string ) Format string with {} placeholders");

  // Add convenient aliases/definitions
  add_definition("PRINTF", "FORMAT PRINT",
                 "( args... format -- ) Format and print string");
}