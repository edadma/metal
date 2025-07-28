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

// static void native_format(context_t* ctx) {
//   require(ctx, 1, "FORMAT");
//   // Get format string
//   cell_t* format_cell = data_pop(ctx);
//   if (format_cell->type != CELL_STRING || !format_cell->payload.utf8) {
//     error(ctx, "FORMAT: format must be a string");
//     return;
//   }
//   const char* format = (char*)format_cell->payload.utf8->data;
//   size_t format_len = format_cell->payload.utf8->length;
//   // Count {} placeholders
//   int placeholder_count = count_placeholders(format, format_len);
//   if (ctx->data_stack_ptr < placeholder_count) {
//     error(ctx, "FORMAT: need %d arguments, have %d", placeholder_count,
//           ctx->data_stack_ptr);
//     return;
//   }
//   // Pop arguments (reverse order since stack is LIFO)
//   cell_t args[placeholder_count];
//   for (int i = placeholder_count - 1; i >= 0; i--) {
//     args[i] = data_pop_cell(ctx);
//   }
//   // Initialize string builder
//   string_builder_t builder;
//   stringbuilder_init(&builder, format_len * 2);
//   size_t pos = 0;
//   int arg_index = 0;
//   char cell_buffer[256];
//   while (pos < format_len) {
//     // Find next '{' or end of string
//     size_t start = pos;
//     while (pos < format_len && format[pos] != '{') {
//       pos++;
//     }
//     // Append everything from start to pos (the literal text chunk)
//     if (pos > start) {
//       stringbuilder_append_string(&builder, format + start, pos - start);
//     }
//     // Handle '{' if we found one
//     if (pos < format_len && format[pos] == '{') {
//       if (pos + 1 < format_len && format[pos + 1] == '{') {
//         // {{ → literal {
//         stringbuilder_append_char(&builder, '{');
//         pos += 2;  // Skip both '{'
//       } else if (pos + 1 < format_len && format[pos + 1] == '}') {
//         // {} → placeholder
//         if (arg_index < placeholder_count) {
//           cell_to_cstr(&args[arg_index], cell_buffer, sizeof(cell_buffer));
//           stringbuilder_append_string(&builder, cell_buffer,
//                                       strlen(cell_buffer));
//           arg_index++;
//         }
//         pos += 2;  // Skip '{}'
//       } else {
//         // Just a '{' - treat as literal
//         stringbuilder_append_char(&builder, '{');
//         pos++;
//       }
//     }
//   }
//   // Get final string and create Metal string cell
//   char* final_str = stringbuilder_end(&builder);
//   cell_t result = string_from_cstr(ctx, final_str);
//   metal_free(final_str);
//
//   data_push(ctx, result);
//
//   // Cleanup
//   release(format_cell);
//   for (int i = 0; i < placeholder_count; i++) {
//     release(&args[i]);
//   }
// }

// cell_t new_preallocated_string(context_t* ctx, char* buffer, size_t length) {
//   // Allocate the string header
//   uint8_array_t* str = metal_alloc(ctx, sizeof(uint8_array_t));
//
//   str->refcount = 1;
//   str->length = length;
//   str->capacity = length;
//   str->data = (uint8_t*)buffer;  // Take ownership of the buffer
//
//   cell_t cell = {0};
//   cell.type = CELL_STRING;
//   cell.payload.utf8 = str;
//   return cell;
// }
//
// void stringbuilder_init(string_builder_t* builder, size_t prefix_size,
//                         size_t capacity_hint) {
//   if (capacity_hint < 16) capacity_hint = 16;
//
//   // Allocate space for prefix + data
//   builder->buffer = metal_malloc(prefix_size + capacity_hint);
//   builder->length = 0;
//   builder->capacity = capacity_hint;
//   builder->prefix_size = prefix_size;
//   // Prefix area is uninitialized - caller fills it in later
// }
//
// void stringbuilder_append_string(string_builder_t* builder, const char* str,
//                                  size_t str_len) {
//   // Ensure capacity
//   while (builder->length + str_len > builder->capacity) {
//     builder->capacity *= 2;
//     builder->buffer = metal_realloc(builder->buffer,
//                                     builder->prefix_size +
//                                     builder->capacity);
//   }
//
//   // Append to data portion (after prefix)
//   char* data_start = (char*)builder->buffer + builder->prefix_size;
//   memcpy(data_start + builder->length, str, str_len);
//   builder->length += str_len;
// }
//
// void stringbuilder_append_char(string_builder_t* builder, char c) {
//   if (builder->length + 1 > builder->capacity) {
//     builder->capacity *= 2;
//     builder->buffer = metal_realloc(builder->buffer,
//                                     builder->prefix_size +
//                                     builder->capacity);
//   }
//
//   char* data_start = (char*)builder->buffer + builder->prefix_size;
//   data_start[builder->length++] = c;
// }
//
// void* stringbuilder_end(string_builder_t* builder) {
//   // Shrink to exact size
//   if (builder->capacity > builder->length) {
//     builder->buffer =
//         metal_realloc(builder->buffer, builder->prefix_size +
//         builder->length);
//   }
//
//   // Return the complete structure (caller fills in prefix)
//   void* result = builder->buffer;
//   builder->buffer = NULL;  // Transfer ownership
//   return result;
// }

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
  // add_definition("PRINTF", "FORMAT PRINT",
  //                "( args... format -- ) Format and print string");
}