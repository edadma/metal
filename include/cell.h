#ifndef CELL_H
#define CELL_H

#include <stddef.h>
#include <stdint.h>

// Forward declaration for circular dependency
typedef struct context context_t;

// Cell types
typedef enum : uint8_t {
  // fundamental types
  CELL_INT32,
  CELL_INT64,
  CELL_FLOAT,
  CELL_STRING,
  CELL_INTERNED,
  CELL_OBJECT,
  CELL_ARRAY,
  CELL_CODE,
  CELL_NATIVE,
  CELL_POINTER,
  CELL_EMPTY,
  CELL_NIL,
  CELL_DATETIME,
  CELL_COORDINATE,
  CELL_COMPLEX,
  CELL_RGB,
  CELL_NULL,
  CELL_UNDEFINED,

  // combined types
  CELL_INT_PAIR,
} cell_type_t;

typedef enum : uint8_t {
  CELL_FLAG_NONE = 0,
  CELL_FLAG_INCOMPLETE = 1 << 0,  // 0x0001
  CELL_FLAG_IMMUTABLE = 1 << 1,   // 0x0002
  CELL_FLAG_WEAK_REF = 1 << 2,    // 0x0004
  CELL_FLAG_TEMPORARY = 1 << 3,   // 0x0008
  CELL_FLAG_IMMEDIATE = 1 << 4,
} cell_flags_t;

typedef void (*native_func_t)(context_t* context);

// Allocated data header (for refcounting)
typedef struct {
  uint32_t refcount;
  // Actual data follows
} alloc_header_t;

#ifdef TARGET_PICO
#pragma pack(push, 4)
#endif

// Metal cell
typedef struct cell {
  cell_type_t type;    // 8 bits
  cell_flags_t flags;  // 8 bits
  uint8_t str_len;     // 8 bits - string length (0-9)
  uint8_t first_char;  // 8 bits - first character (if len > 0)
  union {
    int32_t i32;           // 32-bit integer
    int64_t i64;           // 64-bit integer
    double f;              // Double precision float
    void* ptr;             // Pointer to allocated cell
    native_func_t native;  // Function pointer
    struct cell* pointer;  // Code pointer (using struct tag to avoid issues)
    uint32_t utf32[2];     // 1-2 UTF-32 characters
    struct {
      uint8_t r, g, b;
      uint8_t _pad;  // Align to 4 bytes
    } rgb;
    struct {
      uint32_t timestamp;  // Seconds since Unix epoch (good until 2106)
      int16_t tz_offset;   // Minutes from UTC
      uint16_t _reserved;  // 16 bits left for future use
    } datetime;
    struct {
      float lon, lat;
    } coordinate;
    struct {
      float re, im;
    } complex;
    struct {
      int32_t first, second;
    } int_pair;
  } payload;  // 8 bytes
} cell_t;

#ifdef TARGET_PICO
#pragma pack(pop)
#endif

// Array data structure
typedef struct {
  size_t length;
  size_t capacity;
  cell_t elements[];  // Flexible array member
} cell_array_t;

// Cell creation functions (fundamental immediate types)
cell_t new_int32(int32_t value);
cell_t new_int64(int64_t value);
cell_t new_float(double value);
cell_t new_string(context_t* ctx, const char* utf8);
cell_t new_empty(void);
cell_t new_nil(void);
cell_t new_pointer(cell_t* target);
cell_t new_null(void);
cell_t new_undefined(void);
cell_t new_code(cell_array_t* code_data);

// Cell lifecycle management
void metal_retain(cell_t* cell);
void metal_release(cell_t* cell);

#endif  // CELL_H