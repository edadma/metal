#include "cell.h"

#include <string.h>

#include "compat.h"
#include "debug.h"
#include "error.h"
#include "memory.h"

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
  cell.payload.f64 = value;
  return cell;
}

cell_t new_string(context_t* ctx, const char* utf8) {
  cell_t cell = {0};
  cell.type = CELL_STRING;

  size_t len = strlen(utf8);  // Input length (for now, UTF-8 from C strings)
  uint8_array_t* str = metal_alloc(ctx, sizeof(uint8_array_t) + len);

  str->refcount = 0;  // no owner yet
  str->length = len;
  str->capacity = len;
  memcpy(str->data, utf8, len);

  cell.payload.utf8_ptr = str;
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
  cell.payload.cell_ptr = target;
  return cell;
}

cell_t new_return(cell_t* target) {
  cell_t cell = {0};
  cell.type = CELL_RETURN;
  cell.payload.cell_ptr = target;
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
  cell.payload.array = code_data;
  return cell;
}

cell_t new_boolean(bool value) {
  cell_t cell = {0};
  cell.type = CELL_BOOLEAN;
  cell.payload.boolean = value;
  return cell;
}

void new_boolean_inplace(bool value, cell_t* ptr) {
  cell_t cell = {0};
  cell.type = CELL_BOOLEAN;
  cell.payload.boolean = value;
  *ptr = cell;
}

cell_t new_rgb(uint8_t r, uint8_t g, uint8_t b) {
  cell_t cell = {0};
  cell.type = CELL_RGB;
  cell.payload.rgb.r = r;
  cell.payload.rgb.g = g;
  cell.payload.rgb.b = b;
  return cell;
}

cell_t new_datetime(uint32_t timestamp, int16_t tz_offset) {
  cell_t cell = {0};
  cell.type = CELL_DATETIME;
  cell.payload.datetime.timestamp = timestamp;
  cell.payload.datetime.tz_offset = tz_offset;
  return cell;
}

cell_t new_coordinate(float lon, float lat) {
  cell_t cell = {0};
  cell.type = CELL_COORDINATE;
  cell.payload.coordinate.lon = lon;
  cell.payload.coordinate.lat = lat;
  return cell;
}

cell_t new_complex(float re, float im) {
  cell_t cell = {0};
  cell.type = CELL_COMPLEX;
  cell.payload.complex.re = re;
  cell.payload.complex.im = im;
  return cell;
}

// Cell lifecycle management

void retain(cell_t* cell) {
  if (!cell->payload.ptr) return;

  // Only allocated types need refcount management
  switch (cell->type) {
    case CELL_STRING:
      cell->payload.utf8_ptr->refcount++;
      debug("Retained string, refcount now %d",
            cell->payload.utf8_ptr->refcount);
      break;
    case CELL_OBJECT:
    case CELL_CODE: {
      cell->payload.array->refcount++;
      debug("Retained code, refcount now %d", cell->payload.array->refcount);
      break;
    }
    case CELL_ARRAY: {
      cell->payload.array->refcount++;
      debug("Retained array, refcount now %d", cell->payload.array->refcount);
      break;
    }
    case CELL_POINTER:
      // For pointers, we don't manage the pointed-to memory's refcount
      // The pointer itself doesn't own the memory
      break;
    default:
  }
}

void release(cell_t* cell) {
  if (!cell || !cell->payload.ptr) return;

  switch (cell->type) {
    case CELL_STRING:
      cell->payload.utf8_ptr->refcount--;
      debug("Released string, refcount now %d",
            cell->payload.utf8_ptr->refcount);
      if (cell->payload.utf8_ptr->refcount == 0) {
        metal_free(cell->payload.ptr);
        cell->payload.ptr = NULL;
      }
      break;
    case CELL_OBJECT:
    case CELL_CODE: {
      cell->payload.array->refcount--;
      debug("Released code, refcount now %d", cell->payload.array->refcount);
      if (cell->payload.array->refcount == 0) {
        // Release all elements first
        cell_array_t* data = cell->payload.array;
        for (size_t i = 0; i < data->length; i++) {
          release(&data->elements[i]);
        }
        metal_free(cell->payload.ptr);
        cell->payload.ptr = NULL;
      }
    } break;
    case CELL_ARRAY: {
      cell->payload.array->refcount--;
      debug("Released array cell, refcount now %d",
            cell->payload.array->refcount);
      if (cell->payload.array->refcount == 0) {
        // Release all elements first
        cell_array_t* data = (cell_array_t*)cell->payload.ptr;
        for (size_t i = 0; i < data->length; i++) {
          release(&data->elements[i]);
        }
        metal_free(cell->payload.ptr);
        cell->payload.ptr = NULL;
      }
    } break;
    default:
  }
}