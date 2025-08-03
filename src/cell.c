#include "cell.h"

#include <string.h>

#include "array.h"
#include "compat.h"
#include "debug.h"
#include "error.h"
#include "memory.h"
#include "object.h"

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

cell_t new_allocated_string(context_t* ctx, const string_t* str) {
  cell_t cell = {0};
  cell.type = CELL_STRING;

  // Calculate data size based on encoding
  size_t data_size = str->length;
  if (str->encoding == STRING_UTF16)
    data_size *= 2;
  else if (str->encoding == STRING_UTF32)
    data_size *= 4;

  // Allocate space for allocated_string_t + data
  allocated_string_t* allocated_str = metal_alloc(ctx, sizeof(allocated_string_t) + data_size);

  allocated_str->refcount = 0;  // no owner yet
  allocated_str->string.encoding = str->encoding;
  allocated_str->string.length = str->length;

  // Copy the string data
  memcpy(allocated_str->string.data, str->data, data_size);

  cell.payload.allocated_string = allocated_str;
  return cell;
}

cell_t new_empty_string(void) {
  cell_t cell = {0};
  cell.type = CELL_STRING;
  return cell;
}

cell_t new_empty_object(context_t* ctx) {
  cell_t cell = {0};
  cell.type = CELL_OBJECT;
  cell.payload.object = create_object_data(ctx, 0);  // Always allocate, like arrays
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

cell_t new_code(array_t* code_data) {
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

void retain(const cell_t* cell) {
  if (!cell->payload.ptr) return;  // NULL payload

  // Only allocated types need refcount management
  switch (cell->type) {
    case CELL_STRING:
      if (!(cell->flags & CELL_FLAG_INTERNED)) {
        cell->payload.allocated_string->refcount++;
        debug("Retained string, refcount now %d", cell->payload.allocated_string->refcount);
      }
      break;
    case CELL_OBJECT: {
      cell->payload.object->refcount++;
      debug("Retained object, refcount now %d", cell->payload.object->refcount);
      break;
    }
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
    default:;
  }
}

void release(const cell_t* cell) {
  if (!cell->payload.ptr) return;  // NULL payload

  switch (cell->type) {
    case CELL_STRING:
      if (!(cell->flags & CELL_FLAG_INTERNED)) {
        cell->payload.allocated_string->refcount--;
        debug("Released string, refcount now %d", cell->payload.allocated_string->refcount);
        if (cell->payload.allocated_string->refcount <= 0) {
          metal_free(cell->payload.ptr);
        }
      }
      break;
    case CELL_OBJECT: {
      cell->payload.object->refcount--;
      debug("Released object, refcount now %d", cell->payload.object->refcount);
      if (cell->payload.object->refcount <= 0) {
        free_object_data(cell->payload.object);
      }
    } break;
    case CELL_CODE: {
      cell->payload.array->refcount--;
      debug("Released code, refcount now %d", cell->payload.array->refcount);
      if (cell->payload.array->refcount <= 0) {
        // Release all elements first
        array_t* data = cell->payload.array;
        for (size_t i = 0; i < data->length; i++) {
          release(&data->elements[i]);
        }
        metal_free(cell->payload.ptr);
      }
    } break;
    case CELL_ARRAY: {
      cell->payload.array->refcount--;
      debug("Released array cell, refcount now %d", cell->payload.array->refcount);
      if (cell->payload.array->refcount <= 0) {
        // Release all elements first
        array_t* data = cell->payload.array;

        for (size_t i = 0; i < data->length; i++) {
          release(&data->elements[i]);
        }

        free_array_data(cell->payload.array);
      }
    } break;
    default:;
  }
}