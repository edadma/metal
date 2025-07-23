#include "memory.h"

#include <stdlib.h>

#include "debug.h"
#include "metal.h"

#ifdef TARGET_PICO
#include "pico/mutex.h"
#elifdef TARGET_LINUX
#include <pthread.h>
#endif

// Simple cross-platform mutex wrapper
#ifdef TARGET_WINDOWS
#define LOCK_MEMORY()        // No-op on Windows
#define UNLOCK_MEMORY()      // No-op on Windows
#define INIT_MEMORY_MUTEX()  // No-op on Windows
#elif defined(TARGET_PICO)
static mutex_t memory_mutex;
#define LOCK_MEMORY() mutex_enter_blocking(&memory_mutex)
#define UNLOCK_MEMORY() mutex_exit(&memory_mutex)
#define INIT_MEMORY_MUTEX() mutex_init(&memory_mutex)
#else  // TARGET_LINUX
static pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_MEMORY() pthread_mutex_lock(&memory_mutex)
#define UNLOCK_MEMORY() pthread_mutex_unlock(&memory_mutex)
#define INIT_MEMORY_MUTEX()  // Already initialized
#endif

// Memory management initialization
void init_memory(void) { INIT_MEMORY_MUTEX(); }

// Core allocation functions
void* metal_alloc(size_t size) {
  debug("Allocating %zu bytes", size);

  LOCK_MEMORY();
  alloc_header_t* header = malloc(sizeof(alloc_header_t) + size);

  if (!header) {
    error("Out of memory");
    UNLOCK_MEMORY();
    return NULL;
  }

  header->refcount = 1;
  UNLOCK_MEMORY();
  return (char*)header + sizeof(alloc_header_t);
}

void* metal_realloc(void* ptr, size_t new_size) {
  LOCK_MEMORY();

  if (!ptr) {
    // Just allocate new
    alloc_header_t* header = malloc(sizeof(alloc_header_t) + new_size);
    if (!header) {
      error("Out of memory");
      UNLOCK_MEMORY();
      return NULL;
    }

    header->refcount = 1;
    UNLOCK_MEMORY();
    return (char*)header + sizeof(alloc_header_t);
  }

  alloc_header_t* old_header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  alloc_header_t* new_header =
      realloc(old_header, sizeof(alloc_header_t) + new_size);

  if (!new_header) {
    error("Out of memory");
    UNLOCK_MEMORY();
    return NULL;
  }

  UNLOCK_MEMORY();
  return (char*)new_header + sizeof(alloc_header_t);
}

void metal_free(void* ptr) {
  if (!ptr) return;

  LOCK_MEMORY();
  alloc_header_t* header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  free(header);
  UNLOCK_MEMORY();
}