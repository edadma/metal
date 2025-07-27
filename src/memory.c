#include "memory.h"

#include <stdlib.h>

#include "cell.h"
#include "debug.h"
#include "error.h"
#include "interpreter.h"

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

// Add to src/memory.c

#ifdef TEST_ENABLED
// Memory tracking for unit tests
static int total_allocs = 0;
static int total_frees = 0;

void reset_memory_stats(void) {
  total_allocs = 0;
  total_frees = 0;
}

void get_memory_stats(int* allocs, int* frees) {
  *allocs = total_allocs;
  *frees = total_frees;
}
#endif

void* metal_alloc(context_t* ctx, size_t size) {
  debug("Allocating %zu bytes", size);

  LOCK_MEMORY();
  alloc_header_t* header = malloc(sizeof(alloc_header_t) + size);

  if (!header) {
    error(ctx, "Out of memory");
    UNLOCK_MEMORY();
    return NULL;
  }

  header->refcount = 1;

#ifdef TEST_ENABLED
  total_allocs++;
#endif

  UNLOCK_MEMORY();
  return (char*)header + sizeof(alloc_header_t);
}

void* metal_realloc(context_t* ctx, void* ptr, size_t new_size) {
  LOCK_MEMORY();

  if (!ptr) {
    // Just allocate new
    alloc_header_t* header = malloc(sizeof(alloc_header_t) + new_size);
    if (!header) {
      error(ctx, "Out of memory");
      UNLOCK_MEMORY();
      return NULL;
    }

    header->refcount = 1;

#ifdef TEST_ENABLED
    total_allocs++;  // New allocation
#endif

    UNLOCK_MEMORY();
    return (char*)header + sizeof(alloc_header_t);
  }

  alloc_header_t* old_header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  alloc_header_t* new_header =
      realloc(old_header, sizeof(alloc_header_t) + new_size);

  if (!new_header) {
    error(ctx, "Out of memory");
    UNLOCK_MEMORY();
    return NULL;
  }

  // Note: When ptr is not NULL, this is just resizing existing memory,
  // so no change to allocation/free counts

  UNLOCK_MEMORY();
  return (char*)new_header + sizeof(alloc_header_t);
}

void metal_free(void* ptr) {
  if (!ptr) return;

  LOCK_MEMORY();
  alloc_header_t* header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));

#ifdef TEST_ENABLED
  total_frees++;
#endif

  free(header);
  UNLOCK_MEMORY();
}