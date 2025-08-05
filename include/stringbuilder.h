#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

#include <stddef.h>
#include <stdint.h>

#include "cell.h"
#include "context.h"
#include "strings.h"

// String builder for efficient dynamic string construction
typedef struct {
  uint32_t* utf32;  // allocated intermediate UTF-32 storage
  size_t length;    // number of code points currently stored
  size_t capacity;  // number of code points that can be stored
} string_builder_t;

// Core string builder operations
void stringbuilder_init(context_t* ctx, string_builder_t* builder, size_t initial_capacity);
void stringbuilder_append_codepoint(context_t* ctx, string_builder_t* builder, uint32_t codepoint);
void stringbuilder_append_cstr(context_t* ctx, string_builder_t* builder, const char* utf8_cstr);
void stringbuilder_append_cell(context_t* ctx, string_builder_t* builder, const cell_t* cell, bool observe_base, bool display,
                               const format_spec_t* spec);
string_t* stringbuilder_finalize(context_t* ctx, string_builder_t* builder);
void stringbuilder_cleanup(context_t* ctx, string_builder_t* builder);

#endif  // STRINGBUILDER_H