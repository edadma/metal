#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "debug.h"
#include "error.h"
#include "memory.h"

void skip_whitespace(const char** input_pos) {
  while (**input_pos) {
    // Skip normal whitespace
    if (isspace(**input_pos)) {
      (*input_pos)++;
      continue;
    }

    // Handle // line comments
    if (**input_pos == '/' && *(*input_pos + 1) == '/') {
      // Skip to end of line
      while (**input_pos && **input_pos != '\n') {
        (*input_pos)++;
      }
      // Continue to skip the newline and any following whitespace
      continue;
    }

    // Not whitespace or comment, stop skipping
    break;
  }
}

// Helper function to convert escape sequences
static char process_escape_char(const char c) {
  switch (c) {
    case 'n':
      return '\n';
    case 't':
      return '\t';
    case 'r':
      return '\r';
    case 'b':
      return '\b';
    case 'f':
      return '\f';
    case 'a':
      return '\a';
    case 'v':
      return '\v';
    case '0':
      return '\0';
    case '\\':
      return '\\';
    case '"':
      return '"';
    case '\'':
      return '\'';
    default:
      return c;  // Unknown escape, return as-is
  }
}

// Parse string literal with escape sequence processing
static bool parse_string_literal(const char** input_pos, char* buffer, size_t buffer_size) {
  const char* pos = *input_pos;
  // Skip opening quote
  if (*pos != '"') {
    return false;
  }
  pos++;
  size_t buf_idx = 0;
  while (*pos && *pos != '"') {
    if (buf_idx >= buffer_size - 1) {
      debug("String literal too long");
      return false;
    }
    if (*pos == '\\' && *(pos + 1)) {
      // Escape sequence
      pos++;  // Skip backslash
      buffer[buf_idx++] = process_escape_char(*pos);
      pos++;
    } else {
      // Regular character
      buffer[buf_idx++] = *pos;
      pos++;
    }
  }
  if (*pos != '"') {
    debug("Unterminated string literal");
    return false;
  }

  buffer[buf_idx] = '\0';
  *input_pos = pos + 1;  // Skip closing quote

  debug("Parsed string literal: '%s'", buffer);
  return true;
}

token_type_t parse_next_token(context_t* ctx, const char** input_pos, char* buffer, size_t buffer_size) {
  skip_whitespace(input_pos);
  if (!**input_pos) {
    return TOKEN_EOF;
  }
  // Check for string literal
  if (**input_pos == '"') {
    if (parse_string_literal(input_pos, buffer, buffer_size)) {
      return TOKEN_STRING;
    } else {
      // String parsing failed - this should probably be an error
      return TOKEN_EOF;
    }
  }
  // Parse regular word
  const char* start = *input_pos;
  const char* end = start;
  // Parse until whitespace, quote, or end of input
  while (*end && !isspace(*end)) {
    // Stop at // comment start
    if (*end == '/' && *(end + 1) == '/') {
      break;
    }
    end++;
  }
  size_t length = end - start;
  if (length >= buffer_size) {
    debug("Word too long: %.*s", (int)length, start);
    error(ctx, "Token too long: %.*s", (int)length, start);
  }
  if (length == 0) {
    return TOKEN_EOF;
  }
  strncpy(buffer, start, length);
  buffer[length] = '\0';
  *input_pos = end;
  debug("Parsed word: '%s'", buffer);
  return TOKEN_WORD;
}

buffer_t parse_until_char(context_t* ctx, char delimiter) {
  buffer_t result = {0};

  if (!ctx->input_pos) {
    debug("parse_until_char: not in parsing context");
    return result;
  }

  skip_whitespace(&ctx->input_pos);

  const char* start = ctx->input_pos;
  const char* end = start;

  // Find delimiter
  while (*end && *end != delimiter) {
    end++;
  }

  if (*end != delimiter) {
    debug("parse_until_char: delimiter '%c' not found", delimiter);
    error(ctx, "parse_until_char: delimiter '%c' not found", delimiter);
  }

  // Set up result
  result.ptr = start;
  result.length = end - start;

  // Advance context past delimiter
  ctx->input_pos = end + 1;

  debug("parse_until_char: parsed %zu chars", result.length);
  return result;
}

bool has_more_input(const char* input_pos) {
  skip_whitespace(&input_pos);
  return *input_pos != '\0';
}

void skip_to_end_of_line(const char** input_pos) {
  while (**input_pos && **input_pos != '\n') {
    (*input_pos)++;
  }
  if (**input_pos == '\n') {
    (*input_pos)++;
  }
}