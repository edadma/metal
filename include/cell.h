#ifndef CELL_H
#define CELL_H

#include <stdbool.h>
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
  CELL_BOOLEAN,
  CELL_STRING,
  CELL_INTERNED,
  CELL_OBJECT,
  CELL_ARRAY,
  CELL_CODE,
  CELL_NATIVE,
  CELL_POINTER,
  CELL_RETURN,
  CELL_EMPTY,
  CELL_NIL,
  CELL_DATETIME,
  CELL_COORDINATE,
  CELL_COMPLEX,
  CELL_RGB,
  CELL_NULL,
  CELL_UNDEFINED,
  CELL_BRANCH,           // Unconditional branch
  CELL_BRANCH_IF_FALSE,  // Conditional branch: jump if top of stack is falsy

  // combined types
  CELL_INT_PAIR,
} cell_type_t;

typedef enum : uint8_t {
  CELL_FLAG_IMMEDIATE = 1 << 0,
  CELL_FLAG_INTERNED = 1 << 1,
} cell_flags_t;

typedef void (*native_func_t)(context_t* context);

typedef struct uint8_array {  // extends refcount_t
  int refcount;               // First field - embedded refcount
  size_t length;
  size_t capacity;
  uint8_t data[];  // Variable length string data
} uint8_array_t;

typedef struct uint16_array {  // extends refcount_t
  int refcount;                // First field - embedded refcount
  size_t length;
  size_t capacity;
  uint16_t data[];  // Variable length string data
} uint16_array_t;

typedef struct uint32_array {  // extends refcount_t
  int refcount;                // First field - embedded refcount
  size_t length;
  size_t capacity;
  uint32_t data[];  // Variable length string data
} uint32_array_t;

typedef struct cell_array cell_array_t;

#ifdef TARGET_PICO
#pragma pack(push, 4)
#endif

// Metal cell
typedef struct cell {
  cell_type_t type;    // 8 bits
  cell_flags_t flags;  // 8 bits: [5 bits flags][3 bits str_len]
  int16_t word_idx;    // index of the dictionary word, or -1
  union {
    void* ptr;                  // payload pointer
    int* refcount;              // pointer to embedded refcount
    int32_t i32;                // 32-bit integer
    int64_t i64;                // 64-bit integer
    double f64;                 // Double precision float
    cell_array_t* array;        // Pointer to a cell array
    uint8_array_t* utf8_ptr;    // Pointer to UTF-8 string
    uint16_array_t* utf16_ptr;  // Pointer to UTF-16 string
    uint32_array_t* utf32_ptr;  // Pointer to UTF-32 string
    char utf8[8];               // 0-8 UTF-8 characters
    uint16_t utf16[4];          // 0-4 UTF-16 characters
    uint32_t utf32[2];          // 0-2 UTF-32 characters
    native_func_t native;       // Function pointer
    struct cell* cell_ptr;  // Code pointer (using struct tag to avoid issues)
    struct {
      uint8_t r, g, b;
    } rgb;  // RGB color
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
    bool boolean;
  } payload;  // 8 bytes
} cell_t;

#ifdef TARGET_PICO
#pragma pack(pop)
#endif

#define CELL_STR_LEN(cell) ((cell)->flags_and_len & 0x07)
#define CELL_FLAGS(cell) (((cell)->flags_and_len >> 3) & 0x1F)

#define CELL_SET_STR_LEN(cell, len) \
  ((cell)->flags_and_len = ((cell)->flags_and_len & 0xF8) | ((len) & 0x07))

#define CELL_SET_FLAGS(cell, flags) \
  ((cell)->flags_and_len =          \
       ((cell)->flags_and_len & 0x07) | (((flags) & 0x1F) << 3))

#define CELL_SET_FLAGS_AND_LEN(cell, flags, len) \
  ((cell)->flags_and_len = (((flags) & 0x1F) << 3) | ((len) & 0x07))

// Array data structure
typedef struct cell_array {
  uint32_t refcount;
  int length;
  int capacity;
  cell_t elements[];
} cell_array_t;

// Cell creation functions (fundamental immediate types)
cell_t new_int32(int32_t value);
cell_t new_int64(int64_t value);
cell_t new_float(double value);
cell_t new_string(context_t* ctx, const char* utf8);
cell_t new_empty(void);
cell_t new_nil(void);
cell_t new_pointer(cell_t* target);
cell_t new_return(cell_t* target);
cell_t new_null(void);
cell_t new_undefined(void);
cell_t new_code(cell_array_t* code_data);
cell_t new_boolean(bool value);
void new_boolean_inplace(bool value, cell_t* ptr);
cell_t new_rgb(uint8_t r, uint8_t g, uint8_t b);
cell_t new_datetime(uint32_t timestamp, int16_t tz_offset);
cell_t new_coordinate(float lon, float lat);
cell_t new_complex(float re, float im);

// Cell lifecycle management
void retain(cell_t* cell);
void release(cell_t* cell);

#endif  // CELL_H