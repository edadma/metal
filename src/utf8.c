#include "utf8.h"

#include <ctype.h>
#include <string.h>

// Count UTF-8 characters (not bytes)
size_t utf8_strlen(const char* str) {
  size_t len = 0;
  while (*str) {
    if ((*str & 0xC0) != 0x80) {  // Not a continuation byte
      len++;
    }
    str++;
  }
  return len;
}

// Check if byte is UTF-8 continuation byte
bool utf8_is_continuation_byte(char c) { return (c & 0xC0) == 0x80; }

// Get length of UTF-8 character from first byte
size_t utf8_char_length(char first_byte) {
  if ((first_byte & 0x80) == 0) return 1;     // ASCII
  if ((first_byte & 0xE0) == 0xC0) return 2;  // 2-byte
  if ((first_byte & 0xF0) == 0xE0) return 3;  // 3-byte
  if ((first_byte & 0xF8) == 0xF0) return 4;  // 4-byte
  return 1;                                   // Invalid, treat as single byte
}

// UTF-8 safe strncpy that won't split multi-byte characters
bool utf8_strncpy_safe(char* dest, const char* src, size_t dest_size) {
  if (dest_size == 0) return false;

  size_t pos = 0;
  const char* src_pos = src;

  while (*src_pos && pos < dest_size - 1) {
    size_t char_len = utf8_char_length(*src_pos);

    // Check if we have room for the complete character
    if (pos + char_len >= dest_size) {
      break;  // Would overflow or split character
    }

    // Copy the complete character
    for (size_t i = 0; i < char_len && *src_pos; i++) {
      dest[pos++] = *src_pos++;
    }
  }

  dest[pos] = '\0';
  return *src_pos == '\0';  // True if we copied everything
}

// Case-insensitive UTF-8 comparison (basic ASCII case folding only)
int utf8_stricmp(const char* s1, const char* s2) {
  while (*s1 && *s2) {
    // For ASCII characters, do case-insensitive comparison
    if ((*s1 & 0x80) == 0 && (*s2 & 0x80) == 0) {
      int c1 = tolower(*s1);
      int c2 = tolower(*s2);
      if (c1 != c2) {
        return c1 - c2;
      }
      s1++;
      s2++;
    } else {
      // For non-ASCII, do exact byte comparison
      if (*s1 != *s2) {
        return (unsigned char)*s1 - (unsigned char)*s2;
      }
      s1++;
      s2++;
    }
  }
  return *s1 - *s2;
}