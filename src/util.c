#include "util.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

void print_cell(const cell_t* cell) {
  if (!cell) {
    printf("<null>");
    return;
  }

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
      printf("\"%s\"", cell->payload.utf8_ptr);
      break;
    case CELL_NIL:
      printf("[]");
      break;
    case CELL_ARRAY: {
      const cell_array_t* data = cell->payload.array;

      printf("[");

      for (size_t i = 0; i < data->length; i++) {
        if (i > 0) printf(", ");
        print_cell(&data->elements[i]);
      }

      printf("]");
      break;
    }
    case CELL_POINTER:
      printf("<pointer: ");
      print_cell(cell->payload.cell_ptr);
      printf(">");
      break;
    case CELL_EMPTY:
      printf("<empty>");
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

bool is_truthy(cell_t* cell) {
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
      return val != 0.0 &&
             val == val;  // NaN != NaN, so val == val is false for NaN

    case CELL_STRING:
      if (!cell->payload.utf8_ptr) return false;
      return strlen(cell->payload.utf8_ptr) > 0;

      // Everything else is truthy (including CELL_EMPTY, CELL_NIL, arrays,
      // etc.)
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
        // Check for interned strings first
        if ((a->flags & CELL_FLAG_INTERNED) &&
            (b->flags & CELL_FLAG_INTERNED)) {
          // Both interned - compare pointers
          if (a->payload.utf8_ptr < b->payload.utf8_ptr) return -1;
          if (a->payload.utf8_ptr > b->payload.utf8_ptr) return 1;
          return 0;
        }
        // Lexicographic comparison
        if (!a->payload.utf8_ptr && !b->payload.utf8_ptr) return 0;
        if (!a->payload.utf8_ptr) return -1;
        if (!b->payload.utf8_ptr) return 1;
        return strcmp((char*)a->payload.utf8_ptr, (char*)b->payload.utf8_ptr);
      }

      default:
        error(ctx, "Cannot compare values of this type");
        return 0;
    }
  }

  // Handle numeric type promotion
  if ((a->type == CELL_INT32 || a->type == CELL_INT64 ||
       a->type == CELL_FLOAT) &&
      (b->type == CELL_INT32 || b->type == CELL_INT64 ||
       b->type == CELL_FLOAT)) {
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

// Helper function for equality comparison (works on any types)
bool cells_equal(context_t* ctx, cell_t* a, cell_t* b) {
  if (a->type != b->type) {
    // Different types are only equal if both are numeric and have same value
    if ((a->type == CELL_INT32 || a->type == CELL_INT64) &&
        (b->type == CELL_INT32 || b->type == CELL_INT64)) {
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

    if ((a->type == CELL_INT32 || a->type == CELL_INT64 ||
         a->type == CELL_FLOAT) &&
        (b->type == CELL_INT32 || b->type == CELL_INT64 ||
         b->type == CELL_FLOAT)) {
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
      // Check for interned strings first
      if ((a->flags & CELL_FLAG_INTERNED) && (b->flags & CELL_FLAG_INTERNED)) {
        return a->payload.utf8_ptr == b->payload.utf8_ptr;
      }
      // Regular string comparison
      if (!a->payload.utf8_ptr && !b->payload.utf8_ptr) return true;
      if (!a->payload.utf8_ptr || !b->payload.utf8_ptr) return false;
      return strcmp((char*)a->payload.utf8_ptr, (char*)b->payload.utf8_ptr) ==
             0;

    case CELL_NULL:
    case CELL_UNDEFINED:
    case CELL_EMPTY:
    case CELL_NIL:
      return true;  // These are singletons

    default:
      error(ctx, "unknown cell type: %d", a->type);
  }
}
