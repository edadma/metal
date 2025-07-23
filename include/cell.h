#ifndef CELL_H
#define CELL_H

#include "metal.h"

// Cell creation functions (fundamental immediate types)
cell_t new_int32(int32_t value);
cell_t new_int64(int64_t value);
cell_t new_float(double value);
cell_t new_string(const char* utf8);
cell_t new_empty(void);
cell_t new_nil(void);
cell_t new_pointer(cell_t* target);

// Cell lifecycle management
void metal_retain(cell_t* cell);
void metal_release(cell_t* cell);

#endif  // CELL_H