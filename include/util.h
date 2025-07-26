#ifndef UTIL_H
#define UTIL_H

#include "cell.h"

// Cell printing utility
void print_cell(const cell_t* cell);

int stricmp(const char* s1, const char* s2);

int compare_cells(context_t* ctx, cell_t* a, cell_t* b);
bool is_truthy(cell_t* cell);
bool cells_equal(context_t* ctx, cell_t* a, cell_t* b);

#endif  // UTIL_H