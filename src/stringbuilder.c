#include "stringbuilder.h"

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "memory.h"
#include "strings.h"

// Initialize string builder with initial capacity
void stringbuilder_init(context_t* ctx, string_builder_t* builder, size_t initial_capacity) {
  if (initial_capacity < 16) initial_capacity = 16;  // Minimum reasonable size

  builder->utf32 = metal_alloc(ctx, initial_capacity * sizeof(uint32_t));
  builder->length = 0;
  builder->capacity = initial_capacity;
}

// Ensure builder has at least the specified capacity
static void stringbuilder_ensure_capacity(context_t* ctx, string_builder_t* builder, size_t needed_capacity) {
  if (needed_capacity <= builder->capacity) return;

  // Double until we have enough space
  size_t new_capacity = builder->capacity;
  while (new_capacity < needed_capacity) {
    new_capacity *= 2;
  }

  // Reallocate existing buffer
  builder->utf32 = metal_realloc(ctx, builder->utf32, new_capacity * sizeof(uint32_t));
  builder->capacity = new_capacity;
}

// Append a single Unicode code point
void stringbuilder_append_codepoint(context_t* ctx, string_builder_t* builder, uint32_t codepoint) {
  stringbuilder_ensure_capacity(ctx, builder, builder->length + 1);
  builder->utf32[builder->length++] = codepoint;
}

// Append UTF-8 C string (convert to UTF-32 code points)
void stringbuilder_append_cstr(context_t* ctx, string_builder_t* builder, const char* utf8_cstr) {
  if (!utf8_cstr) return;

  // For now, simple ASCII-only conversion (future: full UTF-8 decode)
  size_t str_len = strlen(utf8_cstr);
  stringbuilder_ensure_capacity(ctx, builder, builder->length + str_len);

  for (size_t i = 0; i < str_len; i++) {
    builder->utf32[builder->length++] = (uint32_t)(unsigned char)utf8_cstr[i];
  }
}

// Append any cell by converting to string representation
void stringbuilder_append_cell(context_t* ctx, string_builder_t* builder, cell_t* cell, bool display, const format_spec_t* spec) {
  if (!cell) error(ctx, "stringbuilder_append_cell: NULL cell pointer");

  char cbuf[32];  // Enough for any float or int64
  string_t* sbuf = NULL;
  size_t size;

  switch (cell->type) {
    // case CELL_INT32: {
    //   snprintf(buffer, sizeof(buffer), "%d", cell->payload.i32);
    //   stringbuilder_append_cstr(ctx, builder, buffer);
    //   break;
    // }
    case CELL_INT32:
      size = sprintf(cbuf, "%d", cell->payload.i32);
      break;

    case CELL_INT64:
      size = sprintf(cbuf, "%lld", (long long)cell->payload.i64);
      break;

    case CELL_FLOAT:
      if (spec && spec->precision >= 0)
        size = sprintf(cbuf, "%.*f", spec->precision, cell->payload.f64);
      else
        size = sprintf(cbuf, "%g", cell->payload.f64);

      break;

    case CELL_BOOLEAN:
      size = sprintf(cbuf, cell->payload.boolean ? "true" : "false");
      break;

    case CELL_NULL:
      size = sprintf(cbuf, "null");
      break;

    case CELL_UNDEFINED:
      size = sprintf(cbuf, "undefined");
      break;

    case CELL_STRING:
      size = string_length(ctx, cell) + (display ? 2 : 0);  // +2 for quotes

      // if (display) stringbuilder_append_codepoint(ctx, builder, '"');
      //
      // // Convert existing string to UTF-32 and append
      // if (!cell->payload.ptr) break;  // Empty string
      //
      // const string_t* str;
      // if (cell->flags & CELL_FLAG_INTERNED) {
      //   str = cell->payload.interned_string;
      // } else {
      //   str = &cell->payload.allocated_string->string;
      // }
      //
      // // For now, simple conversion assuming UTF-8/ASCII (future: proper decode)
      // stringbuilder_ensure_capacity(ctx, builder, builder->length + str->length);
      //
      // for (size_t i = 0; i < str->length; i++) {
      //   builder->utf32[builder->length++] = (uint32_t)str->data[i];
      // }
      //
      // if (display) stringbuilder_append_codepoint(ctx, builder, '"');
      break;

      // case CELL_ARRAY: {
      //   stringbuilder_append_cstr(ctx, builder, "[");
      //   if (cell->payload.array) {
      //     for (size_t i = 0; i < cell->payload.array->length; i++) {
      //       if (i > 0) stringbuilder_append_cstr(ctx, builder, ", ");
      //       stringbuilder_append_cell(ctx, builder, &cell->payload.array->elements[i], true);
      //     }
      //   }
      //   stringbuilder_append_cstr(ctx, builder, "]");
      //   break;
      // }
      //
      // case CELL_OBJECT: {
      //   stringbuilder_append_cstr(ctx, builder, "{");
      //
      //   if (cell->payload.object) {
      //     object_t* obj = cell->payload.object;
      //
      //     for (size_t i = 0; i < obj->length; i++) {
      //       if (i > 0) stringbuilder_append_cstr(ctx, builder, ", ");
      //
      //       // Add key
      //       cell_t key_cell = new_interned_string(obj->pairs[i].key);
      //       stringbuilder_append_cell(ctx, builder, &key_cell, true);  // with quotes
      //
      //       stringbuilder_append_cstr(ctx, builder, ": ");
      //
      //       // Add value
      //       stringbuilder_append_cell(ctx, builder, &obj->pairs[i].value, true);
      //     }
      //   }
      //
      //   stringbuilder_append_cstr(ctx, builder, "}");
      //   break;
      // }
      //
      // case CELL_POINTER:
      //   if (cell->payload.cell_ptr) {
      //     stringbuilder_append_cell(ctx, builder, cell->payload.cell_ptr, display);
      //   } else {
      //     stringbuilder_append_cstr(ctx, builder, "<null pointer>");
      //   }
      //   break;

    default: {
      size = sprintf(cbuf, "<type %d>", cell->type);
      break;
    }
  }

  size_t padding = spec ? spec->width - size : 0;
  size_t left_pad = 0;
  size_t right_pad = padding;

  if (padding > 0 && spec->alignment == ALIGN_RIGHT) {
    left_pad = padding;
    right_pad = 0;
  } else if (padding > 0 && spec->alignment == ALIGN_CENTER) {
    left_pad = padding / 2;
    right_pad = padding - left_pad;
  }

  for (int i = 0; i < left_pad; i++) stringbuilder_append_codepoint(ctx, builder, ' ');

  if (sbuf) {
    // Convert existing string to UTF-32 and append
    stringbuilder_ensure_capacity(ctx, builder, builder->length + sbuf->length);

    for (size_t i = 0; i < sbuf->length; i++) {
      builder->utf32[builder->length++] = (uint32_t)sbuf->data[i];
    }
  } else
    stringbuilder_append_cstr(ctx, builder, cbuf);

  for (int i = 0; i < right_pad; i++) stringbuilder_append_codepoint(ctx, builder, ' ');
}

// Choose optimal encoding for the UTF-32 content
static string_encoding_t choose_optimal_encoding(uint32_t* utf32, size_t length) {
  // For now, always choose UTF-8 (future: analyze content for optimal choice)
  return STRING_UTF8;
}

// Calculate size needed for encoding UTF-32 content to target encoding
static size_t calculate_encoded_size(uint32_t* utf32, size_t length, string_encoding_t encoding) {
  // For now, simple calculation assuming ASCII (future: proper UTF-8 encoding size)
  switch (encoding) {
    case STRING_UTF8:
      return length;  // 1 byte per ASCII character
    case STRING_UTF16:
      return length * 2;
    case STRING_UTF32:
      return length * 4;
    default:
      return length;
  }
}

// Encode UTF-32 content to target encoding
static void encode_utf32_to_target(uint32_t* utf32, size_t length, string_encoding_t encoding, string_t* target) {
  target->encoding = encoding;
  target->length = length;

  // For now, simple conversion assuming ASCII (future: proper UTF-8 encoding)
  switch (encoding) {
    case STRING_UTF8:
      for (size_t i = 0; i < length; i++) {
        target->data[i] = (uint8_t)utf32[i];  // Truncate to ASCII
      }
      break;
    case STRING_UTF16:
      for (size_t i = 0; i < length; i++) {
        ((uint16_t*)target->data)[i] = (uint16_t)utf32[i];
      }
      break;
    case STRING_UTF32:
      memcpy(target->data, utf32, length * sizeof(uint32_t));
      break;
  }
}

// Finalize string builder and return allocated string_t
string_t* stringbuilder_finalize(context_t* ctx, string_builder_t* builder) {
  // Choose optimal encoding
  string_encoding_t optimal = choose_optimal_encoding(builder->utf32, builder->length);

  // Calculate exact size needed
  size_t encoded_size = calculate_encoded_size(builder->utf32, builder->length, optimal);

  // Allocate final string_t + data
  size_t total_size = sizeof(string_t) + encoded_size;
  string_t* final_str = metal_alloc(ctx, total_size);

  // Convert UTF-32 to chosen encoding
  encode_utf32_to_target(builder->utf32, builder->length, optimal, final_str);

  // Free UTF-32 buffer
  metal_free(builder->utf32);
  builder->utf32 = NULL;
  builder->length = 0;
  builder->capacity = 0;

  return final_str;
}

// Cleanup string builder without finalizing (for error cases)
void stringbuilder_cleanup(context_t* ctx, string_builder_t* builder) {
  if (builder->utf32) {
    metal_free(builder->utf32);
    builder->utf32 = NULL;
  }
  builder->length = 0;
  builder->capacity = 0;
}
