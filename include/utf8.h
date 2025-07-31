#ifndef UTF8_H
#define UTF8_H

#include <stdbool.h>
#include <stddef.h>

// UTF-8 utility functions
size_t utf8_strlen(const char* str);
bool utf8_is_continuation_byte(char c);
size_t utf8_char_length(char first_byte);
bool utf8_strncpy_safe(char* dest, const char* src, size_t dest_size);
int utf8_stricmp(const char* s1, const char* s2);

#endif  // UTF8_H