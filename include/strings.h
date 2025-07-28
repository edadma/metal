#ifndef STRINGS_H
#define STRINGS_H

#include "cell.h"
#include "context.h"

// String creation and manipulation
cell_t string_from_cstr(context_t* ctx, const char* cstr);
cell_t string_concat(context_t* ctx, cell_t* a, cell_t* b);
cell_t string_append_cstr(context_t* ctx, cell_t* str, const char* cstr);
cell_t string_append_char(context_t* ctx, cell_t* str, char c);

// String conversion utilities
void cell_to_cstr(cell_t* cell, char* buffer, size_t buffer_size);

// String helper functions
size_t string_length(cell_t* str);
bool string_is_empty(cell_t* str);

void add_string_words(void);

#endif