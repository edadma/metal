#ifndef METAL_H
#define METAL_H

#include <setjmp.h>
#include <stdbool.h>
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

  // combined types
  CELL_INT_PAIR,
} cell_type_t;

typedef enum : uint8_t {
  CELL_FLAG_NONE = 0,
  CELL_FLAG_INCOMPLETE = 1 << 0,  // 0x0001
  CELL_FLAG_IMMUTABLE = 1 << 1,   // 0x0002
  CELL_FLAG_WEAK_REF = 1 << 2,    // 0x0004
  CELL_FLAG_TEMPORARY = 1 << 3,   // 0x0008
} cell_flags_t;

typedef void (*native_func_t)(context_t* context);

#ifdef METAL_TARGET_PICO
#pragma pack(push, 4)
#endif

// Metal cell - 12 bytes (define this first so context can use it)
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

#ifdef METAL_TARGET_PICO
#pragma pack(pop)
#endif

#define DATA_STACK_SIZE 256
#define RETURN_STACK_SIZE 256

// Execution context (now cell_t is complete)
struct context {
  // Stack management
  cell_t data_stack[DATA_STACK_SIZE];
  cell_t return_stack[RETURN_STACK_SIZE];
  int data_stack_ptr;
  int return_stack_ptr;

  // Instruction pointer (for threaded code)
  cell_t** ip;

  // Error handling
  jmp_buf error_jmp;  // For longjmp on errors
  int error_code;
  const char* error_msg;

  // Context identification
  const char* name;  // "REPL", "TIMER_IRQ", etc.
  bool is_interrupt_handler;
};

// Array data structure
typedef struct {
  size_t length;
  size_t capacity;
  cell_t elements[];  // Flexible array member
} array_data_t;

// Allocated data header (for refcounting)
typedef struct {
  uint32_t refcount;
  // Actual data follows
} alloc_header_t;

// Dictionary entry
typedef struct {
  char name[32];      // Word name
  cell_t definition;  // Code cell or other definition
  const char* help;   // Help text (stack effect + description)
} dict_entry_t;

// Interpreter result codes
typedef enum {
  METAL_OK,
  METAL_ERROR,
  METAL_STACK_UNDERFLOW,
  METAL_STACK_OVERFLOW,
  METAL_COMPILE_ERROR,
} metal_result_t;

// Core interpreter functions
metal_result_t metal_interpret(const char* input);
bool metal_input_complete(const char* input);
void metal_error(const char* msg);

// Context management
void metal_init_context(context_t* ctx);
void metal_switch_context(context_t* ctx);
context_t* metal_current_context(void);

// Cell operations
cell_t new_int32(int32_t value);
cell_t new_int64(int64_t value);
cell_t new_float(double value);
cell_t new_string(const char* utf8);
cell_t new_empty(void);
cell_t new_nil(void);
cell_t new_code(void);
cell_t new_native(void);
cell_t new_pointer(cell_t* target);

// Memory management
void* metal_alloc(size_t size);
void* metal_realloc(void* ptr, size_t new_size);
void metal_free(void* ptr);
void metal_retain(cell_t* cell);
void metal_release(cell_t* cell);

// Stack operations
void metal_push(context_t* ctx, cell_t cell);
cell_t metal_pop(context_t* ctx);
cell_t metal_peek(context_t* ctx, int depth);
bool metal_stack_empty(context_t* ctx);

#endif  // METAL_H