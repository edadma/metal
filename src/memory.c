#include "memory.h"

#include <stdlib.h>

#include "debug.h"
#include "metal.h"

#ifdef METAL_TARGET_PICO
#include "pico/mutex.h"
static mutex_t memory_mutex;
#else
#include <pthread.h>
static pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

// Memory management initialization
void init_memory(void) {
#ifdef METAL_TARGET_PICO
  mutex_init(&memory_mutex);
#endif
  debug("Memory management initialized");
}

// Core allocation functions
void* metal_alloc(size_t size) {
  debug("Allocating %zu bytes", size);

#ifdef METAL_TARGET_PICO
  mutex_enter_blocking(&memory_mutex);
#else
  pthread_mutex_lock(&memory_mutex);
#endif

  alloc_header_t* header = malloc(sizeof(alloc_header_t) + size);
  if (!header) {
    error("Out of memory");
#ifdef METAL_TARGET_PICO
    mutex_exit(&memory_mutex);
#else
    pthread_mutex_unlock(&memory_mutex);
#endif
    return NULL;
  }
  header->refcount = 1;

#ifdef METAL_TARGET_PICO
  mutex_exit(&memory_mutex);
#else
  pthread_mutex_unlock(&memory_mutex);
#endif

  return (char*)header + sizeof(alloc_header_t);
}

void* metal_realloc(void* ptr, size_t new_size) {
#ifdef METAL_TARGET_PICO
  mutex_enter_blocking(&memory_mutex);
#else
  pthread_mutex_lock(&memory_mutex);
#endif

  if (!ptr) {
    // Just allocate new
    alloc_header_t* header = malloc(sizeof(alloc_header_t) + new_size);
    if (!header) {
      error("Out of memory");
#ifdef METAL_TARGET_PICO
      mutex_exit(&memory_mutex);
#else
      pthread_mutex_unlock(&memory_mutex);
#endif
      return NULL;
    }
    header->refcount = 1;
#ifdef METAL_TARGET_PICO
    mutex_exit(&memory_mutex);
#else
    pthread_mutex_unlock(&memory_mutex);
#endif
    return (char*)header + sizeof(alloc_header_t);
  }

  alloc_header_t* old_header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  alloc_header_t* new_header =
      realloc(old_header, sizeof(alloc_header_t) + new_size);

  if (!new_header) {
    error("Out of memory");
#ifdef METAL_TARGET_PICO
    mutex_exit(&memory_mutex);
#else
    pthread_mutex_unlock(&memory_mutex);
#endif
    return NULL;
  }

#ifdef METAL_TARGET_PICO
  mutex_exit(&memory_mutex);
#else
  pthread_mutex_unlock(&memory_mutex);
#endif

  return (char*)new_header + sizeof(alloc_header_t);
}

void metal_free(void* ptr) {
  if (!ptr) return;

#ifdef METAL_TARGET_PICO
  mutex_enter_blocking(&memory_mutex);
#else
  pthread_mutex_lock(&memory_mutex);
#endif

  alloc_header_t* header =
      (alloc_header_t*)((char*)ptr - sizeof(alloc_header_t));
  free(header);

#ifdef METAL_TARGET_PICO
  mutex_exit(&memory_mutex);
#else
  pthread_mutex_unlock(&memory_mutex);
#endif
}