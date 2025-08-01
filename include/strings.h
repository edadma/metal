#ifndef STRINGS_H
#define STRINGS_H

#include "cell.h"
#include "context.h"

// String creation and manipulation
cell_t string_concat(context_t* ctx, cell_t* a, cell_t* b);
// cell_t string_append_cstr(context_t* ctx, cell_t* str, const char* cstr);
// cell_t string_append_char(context_t* ctx, cell_t* str, char c);

// String helper functions
size_t string_length(context_t* ctx, cell_t* str);
bool string_is_empty(context_t* ctx, cell_t* str);
bool string_equal(context_t* ctx, const string_t* a, const string_t* b);
void string_from_cstr(const char* cstr, string_t* out_str);

const string_t* intern_lookup(context_t* ctx, const string_t* str);
const string_t* intern_add(context_t* ctx, const string_t* str);
cell_t new_interned_string(const string_t* interned_str);

void add_string_words(void);

typedef struct string_view {
  string_encoding_t encoding;
  size_t length;
  const uint8_t* data;  // Points to data, doesn't own it
} string_view_t;

// String builder for efficient incremental construction
// typedef struct {
//   void* buffer;        // Points to the full allocation (prefix + data)
//   size_t length;       // Current data length
//   size_t capacity;     // Data capacity (excluding prefix)
//   size_t prefix_size;  // Size of reserved prefix space
// } string_builder_t;

// String builder functions
// void stringbuilder_init(string_builder_t* builder, size_t prefix_size, size_t capacity_hint);
// void stringbuilder_append_string(string_builder_t* builder, const char* str, size_t str_len);
// void stringbuilder_append_char(string_builder_t* builder, char c);
// char* stringbuilder_end(string_builder_t* builder);  // Returns final allocated string

// Create string cell from pre-allocated buffer (takes ownership)
cell_t new_preallocated_string(context_t* ctx, char* buffer, size_t length);

// String utility functions (handle interned vs allocated)
const uint8_t* string_data(context_t* ctx, const cell_t* str);
string_view_t string_view(context_t* ctx, const cell_t* str);
bool cell_string_equal(context_t* ctx, const cell_t* a, const cell_t* b);
int string_compare(context_t* ctx, const string_t* a, const string_t* b);

const uint8_t* string_get_data(context_t* ctx, const cell_t* str);
size_t string_get_length(context_t* ctx, const cell_t* str);
string_encoding_t string_get_encoding(context_t* ctx, const cell_t* str);
size_t string_to_utf8(context_t* ctx, const cell_t* str, char* buffer, size_t buffer_size);

int get_intern_count(void);

#endif