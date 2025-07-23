#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

// Memory management initialization
void init_memory(void);

// Core allocation functions
void* metal_alloc(size_t size);
void* metal_realloc(void* ptr, size_t new_size);
void metal_free(void* ptr);

#endif  // MEMORY_H