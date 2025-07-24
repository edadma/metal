#ifndef PARSER_H
#define PARSER_H

#include "metal.h"

// Token types for lexical analysis
typedef enum { TOKEN_WORD, TOKEN_STRING, TOKEN_EOF } token_type_t;

// Core parsing functions
token_type_t parse_next_token(const char** input_pos, char* buffer,
                              size_t buffer_size);
void skip_whitespace(const char** input_pos);
bool has_more_input(const char* input_pos);

// Special parsing for words that need it
char* parse_until_char(context_t* ctx, char delimiter);
void skip_to_end_of_line(const char** input_pos);

#endif  // PARSER_H
