#include "util.h"

#include <ctype.h>
#include <stdio.h>

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
      printf("\"%s\"", (char*)cell->payload.ptr);
      break;
    case CELL_NIL:
      printf("[]");
      break;
    case CELL_ARRAY: {
      const cell_array_t* data = (cell_array_t*)cell->payload.ptr;

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
      print_cell(cell->payload.pointer);
      printf(">");
      break;
    case CELL_EMPTY:
      printf("<empty>");
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
