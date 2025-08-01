#include "util.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "strings.h"

void print_cell(context_t* ctx, const cell_t* cell) {
  switch (cell->type) {
    case CELL_INT32:
      printf("%d", cell->payload.i32);
      break;
    case CELL_INT64:
      printf("%lld", (long long)cell->payload.i64);
      break;
    case CELL_FLOAT:
      printf("%g", cell->payload.f64);
      break;
    case CELL_STRING:
      char buffer[100];

      string_to_utf8(ctx, cell, buffer, sizeof(buffer));
      printf("%s", buffer);
      break;
    case CELL_ARRAY: {
      const cell_array_t* array = cell->payload.array;

      if (!array) {
        printf("[]");
      } else {
        printf("[");

        for (size_t i = 0; i < array->length; i++) {
          if (i > 0) printf(", ");
          print_cell(ctx, &array->elements[i]);
        }

        printf("]");
      }
      break;
    }
    case CELL_POINTER:
      printf("<pointer: ");
      print_cell(ctx, cell->payload.cell_ptr);
      printf(">");
      break;
    case CELL_OBJECT:
      if (!cell->payload.object) {
        printf("{}");
      }
      break;
    case CELL_BOOLEAN:
      printf("%s", cell->payload.boolean ? "true" : "false");
      break;
    case CELL_NULL:
      printf("null");
      break;
    case CELL_UNDEFINED:
      printf("undefined");
      break;
    default:
      printf("<type %d>", cell->type);
      break;
  }
}

// Case-insensitive string comparison
int stricmp(const char* s1, const char* s2) {
  while (*s1 && *s2) {
    int c1 = tolower((unsigned char)*s1);
    int c2 = tolower((unsigned char)*s2);

    if (c1 != c2) return c1 - c2;
    s1++;
    s2++;
  }

  return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

bool is_truthy(context_t* ctx, cell_t* cell) {
  switch (cell->type) {
    case CELL_BOOLEAN:
      return cell->payload.boolean;
    case CELL_NULL:
    case CELL_UNDEFINED:
      return false;
    case CELL_INT32:
      return cell->payload.i32 != 0;
    case CELL_INT64:
      return cell->payload.i64 != 0;
    case CELL_FLOAT:
      // Check for 0.0, -0.0, and NaN
      double val = cell->payload.f64;
      return val != 0.0 && val == val;  // NaN != NaN, so val == val is false for NaN

    case CELL_STRING:
      return !string_is_empty(ctx, cell);
    default:
      return true;
  }
}

// Helper function for comparison operations
// Returns: -1 if a < b, 0 if a == b, 1 if a > b
// Errors for incompatible types
int compare_cells(context_t* ctx, cell_t* a, cell_t* b) {
  // Handle same types first
  if (a->type == b->type) {
    switch (a->type) {
      case CELL_INT32:
        if (a->payload.i32 < b->payload.i32) return -1;
        if (a->payload.i32 > b->payload.i32) return 1;
        return 0;
      case CELL_INT64:
        if (a->payload.i64 < b->payload.i64) return -1;
        if (a->payload.i64 > b->payload.i64) return 1;
        return 0;

      case CELL_FLOAT:
        if (a->payload.f64 < b->payload.f64) return -1;
        if (a->payload.f64 > b->payload.f64) return 1;
        return 0;

      case CELL_STRING: {
        // Handle empty strings
        if (!a->payload.ptr && !b->payload.ptr) return 0;
        if (!a->payload.ptr) return -1;
        if (!b->payload.ptr) return 1;

        // Extract string_t pointers properly
        const string_t* a_str = a->flags & CELL_FLAG_INTERNED ? a->payload.interned_string : &a->payload.allocated_string->string;
        const string_t* b_str = b->flags & CELL_FLAG_INTERNED ? b->payload.interned_string : &b->payload.allocated_string->string;

        return string_compare(ctx, a_str, b_str);
      }
      default:
        error(ctx, "Cannot compare values of this type");
        return 0;
    }
  }

  // Handle numeric type promotion
  if ((a->type == CELL_INT32 || a->type == CELL_INT64 || a->type == CELL_FLOAT) &&
      (b->type == CELL_INT32 || b->type == CELL_INT64 || b->type == CELL_FLOAT)) {
    // Convert both to the "highest" numeric type
    double a_val, b_val;

    // Convert a to double
    switch (a->type) {
      case CELL_INT32:
        a_val = (double)a->payload.i32;
        break;
      case CELL_INT64:
        a_val = (double)a->payload.i64;
        break;
      case CELL_FLOAT:
        a_val = a->payload.f64;
        break;
      default:
        a_val = 0;
        break;  // Should never happen
    }

    // Convert b to double
    switch (b->type) {
      case CELL_INT32:
        b_val = (double)b->payload.i32;
        break;
      case CELL_INT64:
        b_val = (double)b->payload.i64;
        break;
      case CELL_FLOAT:
        b_val = b->payload.f64;
        break;
      default:
        b_val = 0;
        break;  // Should never happen
    }

    if (a_val < b_val) return -1;
    if (a_val > b_val) return 1;
    return 0;
  }

  error(ctx, "Cannot compare incompatible types");
  return 0;
}

// Helper function for equality comparison (works on any type)
bool cells_equal(context_t* ctx, cell_t* a, cell_t* b) {
  if (a->type != b->type) {
    // Different types are only equal if both are numeric and have same value
    if ((a->type == CELL_INT32 || a->type == CELL_INT64) && (b->type == CELL_INT32 || b->type == CELL_INT64)) {
      int64_t a_val, b_val;

      switch (a->type) {
        case CELL_INT32:
          a_val = (int64_t)a->payload.i32;
          break;
        default:
          a_val = a->payload.i64;
          break;
      }

      switch (b->type) {
        case CELL_INT32:
          b_val = (int64_t)b->payload.i32;
          break;
        default:
          b_val = b->payload.i64;
          break;
      }

      return a_val == b_val;
    }

    if ((a->type == CELL_INT32 || a->type == CELL_INT64 || a->type == CELL_FLOAT) &&
        (b->type == CELL_INT32 || b->type == CELL_INT64 || b->type == CELL_FLOAT)) {
      double a_val, b_val;

      switch (a->type) {
        case CELL_INT32:
          a_val = (double)a->payload.i32;
          break;
        case CELL_INT64:
          a_val = (double)a->payload.i64;
          break;
        default:
          a_val = a->payload.f64;
          break;
      }

      switch (b->type) {
        case CELL_INT32:
          b_val = (double)b->payload.i32;
          break;
        case CELL_INT64:
          b_val = (double)b->payload.i64;
          break;
        default:
          b_val = b->payload.f64;
          break;
      }

      return a_val == b_val;
    }

    return false;
  }

  // Same types
  switch (a->type) {
    case CELL_INT32:
      return a->payload.i32 == b->payload.i32;

    case CELL_INT64:
      return a->payload.i64 == b->payload.i64;

    case CELL_FLOAT:
      return a->payload.f64 == b->payload.f64;

    case CELL_BOOLEAN:
      return a->payload.boolean == b->payload.boolean;

    case CELL_STRING:
      return cell_string_equal(ctx, a, b);
    case CELL_NULL:
    case CELL_UNDEFINED:
      return true;  // These are singletons
    default:
      error(ctx, "Cannot compare values of this type");
  }
}

void dump(const void* data, size_t size) {
  const unsigned char* bytes = (const unsigned char*)data;
  size_t i;

  printf("Address   : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | ASCII\n");
  printf("----------+------------------------------------------------+------------------\n");

  for (i = 0; i < size; i += 16) {
    // Print address
    printf("%08zX  : ", i);

    // Print hex bytes
    for (size_t j = 0; j < 16; j++) {
      if (i + j < size) {
        printf("%02X ", bytes[i + j]);
      } else {
        printf("   ");
      }
    }

    printf("| ");

    // Print ASCII representation
    for (size_t j = 0; j < 16 && (i + j) < size; j++) {
      unsigned char c = bytes[i + j];
      printf("%c", isprint(c) ? c : '.');
    }

    printf("\n");
  }
  printf("\n");
}

int join(context_t* ctx, cell_t cell[], size_t length, const char* sep, char* buffer, size_t buffer_size) {
  if (!buffer || buffer_size == 0) {
    return 0;
  }

  int size = 0;

  if (length == 0) {
    return size;
  }

  for (size_t i = 0; i < length; i++) {
    if (i > 0) size += snprintf(buffer + size, buffer_size - size, "%s", sep);
    size += cell_to_cstr(ctx, &cell[i], buffer + size, buffer_size - size);
  }

  return size;
}

// Convert cell to C string representation
int cell_to_cstr(context_t* ctx, cell_t* cell, char* buffer, size_t buffer_size) {
  if (!cell || !buffer || buffer_size == 0) {
    return 0;
  }

  switch (cell->type) {
    case CELL_INT32:
      return snprintf(buffer, buffer_size, "%d", cell->payload.i32);

    case CELL_INT64:
      return snprintf(buffer, buffer_size, "%lld", (long long)cell->payload.i64);

    case CELL_FLOAT:
      return snprintf(buffer, buffer_size, "%g", cell->payload.f64);

    case CELL_STRING:
      char buf[100];

      string_to_utf8(ctx, cell, buf, sizeof(buf));
      return snprintf(buffer, buffer_size, "%s", buf);

    case CELL_BOOLEAN:
      return snprintf(buffer, buffer_size, "%s", cell->payload.boolean ? "true" : "false");

    case CELL_ARRAY:
      int size = snprintf(buffer, buffer_size, "[", 1);

      size += join(ctx, cell->payload.array->elements, cell->payload.array->length, ", ", buffer + size, buffer_size - size);
      return size + snprintf(buffer + size, buffer_size - size, "]");

    case CELL_OBJECT:
      if (!cell->payload.object) {
        return snprintf(buffer, buffer_size, "{}");
      }

      return snprintf(buffer, buffer_size, "{object:%d}", (int)cell->payload.object->length);

    case CELL_POINTER:
      return cell_to_cstr(ctx, cell->payload.cell_ptr, buffer, buffer_size);

    case CELL_NULL:
      return snprintf(buffer, buffer_size, "null");

    case CELL_UNDEFINED:
      return snprintf(buffer, buffer_size, "undefined");

    default:
      return snprintf(buffer, buffer_size, "<type %d>", cell->type);
  }
}
