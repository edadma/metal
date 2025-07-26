#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#include "context.h"

// Memory management initialization
void init_memory(void);

// Core allocation functions
void* metal_alloc(context_t* ctx, size_t size);
void* metal_realloc(context_t* ctx, void* ptr, size_t new_size);
void metal_free(void* ptr);

#endif  // MEMORY_H