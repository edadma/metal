#ifndef STRINGS_H
#define STRINGS_H

#include "cell.h"
#include "context.h"

typedef struct {
  int width;      // minimum width (0 = no width specified)
  int precision;  // decimal places (-1 = not specified)
  bool hex;       // use hex formatting
  enum {
    ALIGN_LEFT,   // default (no prefix)
    ALIGN_RIGHT,  // > prefix
    ALIGN_CENTER  // ^ prefix
  } alignment;
} format_spec_t;

// String helper functions
size_t string_length(context_t* ctx, const cell_t* str);
bool string_is_empty(context_t* ctx, cell_t* str);
bool string_equal(const string_t* a, const string_t* b);
void string_from_cstr(const char* cstr, string_t* out_str);

const string_t* intern_lookup(context_t* ctx, const string_t* str);
const string_t* intern_add(context_t* ctx, const string_t* str);
cell_t new_interned_string(const string_t* interned_str);
void clear_intern_table(void);

void add_string_words(void);

typedef struct string_view {
  string_encoding_t encoding;
  size_t length;
  const uint8_t* data;  // Points to data, doesn't own it
} string_view_t;

// String utility functions (handle interned vs allocated)
string_view_t string_view(context_t* ctx, const cell_t* str);
bool cell_string_equal(context_t* ctx, const cell_t* a, const cell_t* b);
int string_compare(context_t* ctx, const string_t* a, const string_t* b);

const uint8_t* string_get_data(context_t* ctx, const cell_t* str);
size_t string_get_length(context_t* ctx, const cell_t* str);
string_encoding_t string_get_encoding(context_t* ctx, const cell_t* str);
size_t string_to_utf8(context_t* ctx, const cell_t* str, char* buffer, size_t buffer_size);
uint32_t string_char_at(context_t* ctx, const string_t* str, size_t index);

int get_intern_count(void);

#endif