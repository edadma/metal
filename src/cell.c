#include "cell.h"

#include <string.h>

#include "debug.h"
#include "memory.h"
#include "metal.h"

// Cell creation functions (fundamental immediate types)

cell_t new_int32(int32_t value) {
  cell_t cell = {0};
  cell.type = CELL_INT32;
  cell.payload.i32 = value;
  return cell;
}

cell_t new_int64(int64_t value) {
  cell_t cell = {0};
  cell.type = CELL_INT64;
  cell.payload.i64 = value;
  return cell;
}

cell_t new_float(double value) {
  cell_t cell = {0};
  cell.type = CELL_FLOAT;
  cell.payload.f = value;
  return cell;
}

cell_t new_string(const char* utf8) {
  cell_t cell = {0};
  cell.type = CELL_STRING;

  size_t len = strlen(utf8);

  // For now, just allocate all strings. Later we'll optimize for short ones
  char* allocated = metal_alloc(len + 1);
  if (!allocated) {
    // Return empty on allocation failure
    return new_empty();
  }
  strcpy(allocated, utf8);
  cell.payload.ptr = allocated;

  return cell;
}

cell_t new_empty(void) {
  cell_t cell = {0};
  cell.type = CELL_EMPTY;
  return cell;
}

cell_t new_nil(void) {
  cell_t cell = {0};
  cell.type = CELL_NIL;
  return cell;
}

cell_t new_pointer(cell_t* target) {
  cell_t cell = {0};
  cell.type = CELL_POINTER;
  cell.payload.pointer = target;
  return cell;
}

cell_t new_null(void) {
  cell_t cell = {0};
  cell.type = CELL_NULL;
  return cell;
}

cell_t new_undefined(void) {
  cell_t cell = {0};
  cell.type = CELL_UNDEFINED;
  return cell;
}

cell_t new_code(cell_array_t* code_data) {
  cell_t cell = {0};
  cell.type = CELL_CODE;
  cell.payload.ptr = code_data;
  return cell;
}

// Cell lifecycle management

void metal_retain(cell_t* cell) {
  if (!cell || !cell->payload.ptr) return;

  // Only allocated types need refcount management
  switch (cell->type) {
    case CELL_STRING:
    case CELL_OBJECT:
    case CELL_CODE:
      if (!(cell->flags & CELL_FLAG_WEAK_REF)) {
        alloc_header_t* header = (alloc_header_t*)((char*)cell->payload.ptr -
                                                   sizeof(alloc_header_t));
        header->refcount++;
        debug("Retained cell type %d, refcount now %d", cell->type,
              header->refcount);
      }
      break;
    case CELL_ARRAY:
      if (!(cell->flags & CELL_FLAG_WEAK_REF)) {
        alloc_header_t* header = (alloc_header_t*)((char*)cell->payload.ptr -
                                                   sizeof(alloc_header_t));
        header->refcount++;
        debug("Retained array cell, refcount now %d", header->refcount);
      }
      break;
    case CELL_POINTER:
      // For pointers, we don't manage the pointed-to memory's refcount
      // The pointer itself doesn't own the memory
      break;
    default:
      break;
  }
}

void metal_release(cell_t* cell) {
  if (!cell || !cell->payload.ptr) return;

  switch (cell->type) {
    case CELL_STRING:
    case CELL_OBJECT:
    case CELL_CODE:
      if (!(cell->flags & CELL_FLAG_WEAK_REF)) {
        alloc_header_t* header = (alloc_header_t*)((char*)cell->payload.ptr -
                                                   sizeof(alloc_header_t));
        header->refcount--;
        debug("Released cell type %d, refcount now %d", cell->type,
              header->refcount);
        if (header->refcount == 0) {
          metal_free(cell->payload.ptr);
          cell->payload.ptr = NULL;
        }
      }
      break;
    case CELL_ARRAY:
      if (!(cell->flags & CELL_FLAG_WEAK_REF)) {
        alloc_header_t* header = (alloc_header_t*)((char*)cell->payload.ptr -
                                                   sizeof(alloc_header_t));
        header->refcount--;
        debug("Released array cell, refcount now %d", header->refcount);
        if (header->refcount == 0) {
          // Release all elements first
          cell_array_t* data = (cell_array_t*)cell->payload.ptr;
          for (size_t i = 0; i < data->length; i++) {
            metal_release(&data->elements[i]);
          }
          metal_free(cell->payload.ptr);
          cell->payload.ptr = NULL;
        }
      }
      break;
    case CELL_POINTER:
      // Pointers don't own the pointed-to memory
      break;
    default:
      break;
  }
}