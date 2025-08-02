#include "strings.h"

#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "error.h"
#include "interpreter.h"
#include "memory.h"
#include "require.h"
#include "stack.h"
#include "stringbuilder.h"

typedef struct intern intern_t;
typedef struct intern {
  const string_t* string;
  const intern_t* next;
} intern_t;

static intern_t* intern_list = NULL;

const string_t* intern_lookup(context_t* ctx, const string_t* str) {
  for (const intern_t* node = intern_list; node; node = node->next) {
    if (string_equal(ctx, str, node->string)) {
      return node->string;
    }
  }

  return NULL;  // Not found
}

const string_t* intern_add(context_t* ctx, const string_t* str) {
  // Calculate size needed for string_t + its data
  size_t data_size = str->length;
  if (str->encoding == STRING_UTF16)
    data_size *= 2;
  else if (str->encoding == STRING_UTF32)
    data_size *= 4;

  size_t total_size = sizeof(string_t) + data_size;

  // Allocate permanent copy of the string_t
  string_t* permanent_str = metal_alloc(ctx, total_size);

  // Copy the header
  permanent_str->encoding = str->encoding;
  permanent_str->length = str->length;

  // Copy the data
  memcpy(permanent_str->data, str->data, data_size);

  // Allocate and link the intern_t node
  intern_t* node = metal_alloc(ctx, sizeof(intern_t));
  node->string = permanent_str;
  node->next = intern_list;
  intern_list = node;

  return permanent_str;  // Return the permanent copy
}

cell_t new_interned_string(const string_t* interned_str) {
  cell_t cell = {0};
  cell.type = CELL_STRING;
  cell.flags = CELL_FLAG_INTERNED;
  cell.payload.interned_string = interned_str;
  return cell;
}

void string_from_cstr(const char* cstr, string_t* out_str) {
  size_t len = strlen(cstr);

  // For now, simple UTF-8 copy (future: handle encoding conversion)
  out_str->encoding = STRING_UTF8;
  out_str->length = len;

  // Copy string data into the flexible array
  memcpy(out_str->data, cstr, len);
}

// Get string length (handles NULL payload)
size_t string_length(context_t* ctx, cell_t* str) {
  require(ctx, str != NULL);
  require(ctx, str->type == CELL_STRING);

  if (!str->payload.ptr) return 0;  // empty string

  if (str->flags & CELL_FLAG_INTERNED) return str->payload.interned_string->length;

  return str->payload.allocated_string->string.length;
}

// Check if string is empty
bool string_is_empty(context_t* ctx, cell_t* str) { return string_length(ctx, str) == 0; }

// Concatenate two strings (handles empty strings with NULL payloads)
cell_t string_concat(context_t* ctx, cell_t* a, cell_t* b) {
  if (a->type != CELL_STRING || b->type != CELL_STRING) {
    error(ctx, "string_concat: both arguments must be strings");
  }

  size_t alen = string_length(ctx, a);
  size_t blen = string_length(ctx, b);

  // If both empty, return empty string
  if (alen == 0 && blen == 0) {
    return new_empty_string();
  }

  // If one is empty, return copy of the other
  if (alen == 0) {
    return *b;
  }

  if (blen == 0) {
    return *a;
  }

  // Both have content - allocate new string
  allocated_string_t* new_str = metal_alloc(ctx, sizeof(allocated_string_t) + alen + blen);

  // Initialize the new string
  new_str->refcount = 1;
  new_str->string.encoding = STRING_UTF8;
  new_str->string.length = alen + blen;

  // Copy both strings
  const uint8_t* adata = string_get_data(ctx, a);
  const uint8_t* bdata = string_get_data(ctx, b);

  memcpy(new_str->string.data, adata, alen);
  memcpy(new_str->string.data + alen, bdata, blen);

  // Create result cell
  cell_t result = {0};
  result.type = CELL_STRING;
  result.payload.allocated_string = new_str;

  return result;
}

// String operations for Metal language

static void native_string_empty_q(context_t* ctx) {
  require_params(ctx, 1, "STRING-EMPTY?");
  cell_t* str = data_pop(ctx);

  if (str->type != CELL_STRING) {
    error(ctx, "STRING-EMPTY?: argument must be string");
  }

  bool empty = string_is_empty(ctx, str);
  release(str);
  data_push(ctx, new_boolean(empty));
}

// String utility functions to handle interned vs allocated strings

// Get both length and data in one call (more efficient)
string_view_t string_view(context_t* ctx, const cell_t* str) {
  require(ctx, str != NULL);
  require(ctx, str->type == CELL_STRING);
  string_view_t view = {0};
  if (!str->payload.ptr) {
    // Empty string
    view.encoding = STRING_UTF8;  // Default encoding for empty strings
    view.length = 0;
    view.data = (const uint8_t*)"";
    return view;
  }

  if (str->flags & CELL_FLAG_INTERNED) {
    view.encoding = str->payload.interned_string->encoding;
    view.length = str->payload.interned_string->length;
    view.data = str->payload.interned_string->data;
  } else {
    view.encoding = str->payload.allocated_string->string.encoding;
    view.length = str->payload.allocated_string->string.length;
    view.data = str->payload.allocated_string->string.data;
  }

  return view;
}

bool string_equal(context_t* ctx, const string_t* a, const string_t* b) {
  if (a->length != b->length) return false;
  require_msg(ctx, a->encoding == b->encoding, "string_equal: encoding mismatch");

  size_t len = a->length;

  if (a->encoding == STRING_UTF16)
    len <<= 1;
  else if (a->encoding == STRING_UTF32)
    len <<= 2;

  return memcmp(a->data, b->data, len) == 0;
}

// String comparison for ordering (like strcmp)
// Returns: -1 if a < b, 0 if a == b, 1 if a > b
int string_compare(context_t* ctx, const string_t* a, const string_t* b) {
  require_msg(ctx, a->encoding == b->encoding, "string_compare: encoding mismatch");

  // Calculate byte lengths based on encoding
  size_t alen = a->length;
  size_t blen = b->length;

  if (a->encoding == STRING_UTF16) {
    alen <<= 1;
    blen <<= 1;
  } else if (a->encoding == STRING_UTF32) {
    alen <<= 2;
    blen <<= 2;
  }

  // Compare the actual string contents
  size_t min_len = (alen < blen) ? alen : blen;
  int result = memcmp(a->data, b->data, min_len);

  if (result != 0) {
    return (result < 0) ? -1 : 1;
  }

  // Same content up to min_len, decide by length
  if (alen < blen) return -1;
  if (alen > blen) return 1;
  return 0;
}

// Fast string equality with interned string optimization
bool cell_string_equal(context_t* ctx, const cell_t* a, const cell_t* b) {
  require(ctx, a != NULL);
  require(ctx, b != NULL);
  require(ctx, a->type == CELL_STRING);
  require(ctx, b->type == CELL_STRING);

  // Fast path: both interned means pointer comparison
  if ((a->flags & CELL_FLAG_INTERNED) && (b->flags & CELL_FLAG_INTERNED)) {
    return a->payload.interned_string == b->payload.interned_string;
  }

  // Same pointer optimization (covers both empty strings AND same allocated string)
  if (a->payload.ptr == b->payload.ptr) return true;

  // One empty, one not empty
  if (!a->payload.ptr || !b->payload.ptr) return false;

  // Now safe to extract string_t pointers
  const string_t* a_str = a->flags & CELL_FLAG_INTERNED ? a->payload.interned_string : &a->payload.allocated_string->string;
  const string_t* b_str = b->flags & CELL_FLAG_INTERNED ? b->payload.interned_string : &b->payload.allocated_string->string;

  return string_equal(ctx, a_str, b_str);
}

// Get string data pointer (handles interned vs allocated)
const uint8_t* string_get_data(context_t* ctx, const cell_t* str) {
  require(ctx, str != NULL);
  require(ctx, str->type == CELL_STRING);

  if (!str->payload.ptr) return (const uint8_t*)"";  // empty string

  if (str->flags & CELL_FLAG_INTERNED) {
    return str->payload.interned_string->data;
  } else {
    return str->payload.allocated_string->string.data;
  }
}

// Get string length (handles interned vs allocated)
size_t string_get_length(context_t* ctx, const cell_t* str) {
  require(ctx, str != NULL);
  require(ctx, str->type == CELL_STRING);

  if (!str->payload.ptr) return 0;  // empty string

  if (str->flags & CELL_FLAG_INTERNED) {
    return str->payload.interned_string->length;
  } else {
    return str->payload.allocated_string->string.length;
  }
}

// Get string encoding (handles interned vs allocated)
string_encoding_t string_get_encoding(context_t* ctx, const cell_t* str) {
  require(ctx, str != NULL);
  require(ctx, str->type == CELL_STRING);

  if (!str->payload.ptr) return STRING_UTF8;  // empty string default

  if (str->flags & CELL_FLAG_INTERNED) {
    return str->payload.interned_string->encoding;
  } else {
    return str->payload.allocated_string->string.encoding;
  }
}

// Convert string to UTF-8 for printing (for now assumes input is UTF-8)
// Returns: number of bytes written to buffer (not including null terminator)
size_t string_to_utf8(context_t* ctx, const cell_t* str, char* buffer, size_t buffer_size) {
  require(ctx, str != NULL);
  require(ctx, str->type == CELL_STRING);
  require(ctx, buffer != NULL);
  require(ctx, buffer_size > 0);

  if (!str->payload.ptr) {
    // Empty string
    if (buffer_size > 0) buffer[0] = '\0';
    return 0;
  }

  const uint8_t* data = string_get_data(ctx, str);
  size_t length = string_get_length(ctx, str);
  string_encoding_t encoding = string_get_encoding(ctx, str);

  // For now, assume all strings are UTF-8
  // TODO: Add actual UTF-16/32 to UTF-8 conversion
  if (encoding != STRING_UTF8) {
    error(ctx, "string_to_utf8: UTF-16/32 conversion not yet implemented");
  }

  size_t copy_len = (length < buffer_size - 1) ? length : buffer_size - 1;
  memcpy(buffer, data, copy_len);
  buffer[copy_len] = '\0';

  return copy_len;
}

int get_intern_count(void) {
  int count = 0;
  for (const intern_t* node = intern_list; node; node = node->next) {
    count++;
  }
  return count;
}

// Get codepoint at index from string_t (regardless of encoding)
uint32_t string_char_at(context_t* ctx, const string_t* str, size_t index) {
  if (index >= str->length) {
    error(ctx, "string index out of bounds");
  }

  switch (str->encoding) {
    case STRING_UTF8:
      return (uint32_t)str->data[index];  // Simple for now (ASCII)
    case STRING_UTF16:
      return (uint32_t)((uint16_t*)str->data)[index];
    case STRING_UTF32:
      return ((uint32_t*)str->data)[index];
    default:
      error(ctx, "unknown string encoding");
  }
}

// FORMAT implementation - no buffer restrictions!
// Supports {} placeholders and {{ for literal {
void native_format(context_t* ctx) {
  require_params(ctx, 1, "FORMAT");

  // Get format string from top
  cell_t* format_cell = data_pop(ctx);
  if (format_cell->type != CELL_STRING) {
    error(ctx, "FORMAT: format must be a string");
  }

  // Get the actual string_t regardless of interned/allocated
  const string_t* format_str;
  if (format_cell->flags & CELL_FLAG_INTERNED) {
    format_str = format_cell->payload.interned_string;
  } else {
    format_str = &format_cell->payload.allocated_string->string;
  }

  string_builder_t builder;
  stringbuilder_init(ctx, &builder, format_str->length + 64);

  int args_used = 0;

  for (size_t pos = 0; pos < format_str->length; pos++) {
    uint32_t c = string_char_at(ctx, format_str, pos);

    if (c != '{') {
      // Regular character - append directly
      stringbuilder_append_codepoint(ctx, &builder, c);
    } else {
      // Found '{' - check what follows
      if (pos + 1 >= format_str->length) {
        error(ctx, "FORMAT: unterminated placeholder");
      }

      uint32_t next = string_char_at(ctx, format_str, pos + 1);
      if (next == '{') {
        // {{ -> literal {
        stringbuilder_append_codepoint(ctx, &builder, '{');
        pos++;  // Skip the second {
      } else if (next == '}') {
        // {} -> placeholder
        if (ctx->data_stack_ptr <= args_used) {
          error(ctx, "FORMAT: insufficient stack");
        }

        // Peek at next argument and append it
        cell_t* arg = data_peek(ctx, args_used);
        stringbuilder_append_cell(ctx, &builder, arg);
        args_used++;
        pos++;  // Skip the }
      } else {
        error(ctx, "FORMAT: invalid placeholder syntax");
      }
    }
  }

  // Finalize the string
  string_t* result_str = stringbuilder_finalize(ctx, &builder);

  // Pop and release the arguments we consumed
  for (int i = 0; i < args_used; i++) {
    cell_t* popped = data_pop(ctx);
    release(popped);
  }

  // Release format string and push result
  release(format_cell);
  data_push(ctx, new_allocated_string(ctx, result_str));
}

// Register all string words
void add_string_words(void) {
  add_native_word("STRING-EMPTY?", native_string_empty_q, "( string -- bool ) Test if string is empty");
  add_native_word("FORMAT", native_format, "( args... format -- string ) Format string with {} placeholders");

  // Add convenient aliases/definitions
  add_definition("PRINTF", "FORMAT PRINT", "( args... format -- ) Format and print string");
}
