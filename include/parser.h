#ifndef PARSER_H
#define PARSER_H

#include "metal.h"

// Core parsing functions
bool parse_next_word(const char** input_pos, char* buffer, size_t buffer_size);
void skip_whitespace(const char** input_pos);
bool has_more_input(const char* input_pos);

// Special parsing for words that need it
char* parse_until_char(context_t* ctx, char delimiter);
void skip_to_end_of_line(const char** input_pos);

#endif  // PARSER_H
