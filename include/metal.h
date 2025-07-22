#ifndef METAL_H
#define METAL_H

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

// Cell types
typedef enum {
    CELL_INT32,
    CELL_INT64,
    CELL_FLOAT,
    CELL_STRING,
    CELL_OBJECT,
    CELL_ARRAY,
    CELL_CODE,
    CELL_POINTER,
    CELL_EMPTY,
    CELL_NIL
} cell_type_t;

// Metal cell - 12 bytes
typedef struct {
    cell_type_t type;        // 4 bytes
    union {
        int32_t i32;         // 32-bit integer
        int64_t i64;         // 64-bit integer
        double f;            // Double precision float
        void* ptr;           // Pointer to allocated data
        uint32_t utf32[2];   // 1-2 UTF-32 characters
    } payload;               // 8 bytes
} cell_t;

// Allocated data header (for refcounting)
typedef struct {
    uint32_t refcount;
    // Actual data follows
} alloc_header_t;

// Execution context
typedef struct {
    // Stack management
    cell_t* data_stack;
    cell_t* data_top;
    cell_t* data_limit;

    cell_t* return_stack;
    cell_t* return_top;
    cell_t* return_limit;

    // Instruction pointer (for threaded code)
    void** ip;

    // Error handling
    jmp_buf error_jmp;      // For longjmp on errors
    int error_code;
    const char* error_msg;

} execution_context_t;

// Dictionary entry
typedef struct {
    char name[32];          // Word name
    cell_t definition;      // Code cell or other definition
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
void metal_init_context(execution_context_t* ctx);
void metal_switch_context(execution_context_t* ctx);
execution_context_t* metal_current_context(void);

// Cell operations
cell_t new_int32(int32_t value);
cell_t new_int64(int64_t value);
cell_t new_float(double value);
cell_t new_string(const char* str);
cell_t new_empty_object(void);
cell_t new_nil(void);

// Memory management
void* metal_alloc(size_t size);
void metal_free(void* ptr);
void metal_retain(cell_t* cell);
void metal_release(cell_t* cell);

// Stack operations
void metal_push(cell_t cell);
cell_t metal_pop(void);
cell_t metal_peek(int depth);
bool metal_stack_empty(void);

#endif // METAL_H