#include "util.h"

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
      printf("%g", cell->payload.f);
      break;
    case CELL_STRING:
      printf("\"%s\"", (char*)cell->payload.ptr);
      break;
    case CELL_NIL:
      printf("[]");
      break;
    case CELL_ARRAY: {
      array_data_t* data = (array_data_t*)cell->payload.ptr;
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