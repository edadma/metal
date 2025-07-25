#ifndef CORE_H
#define CORE_H

#include "cell.h"

extern bool compilation_mode;

// Add core language words to the dictionary
void add_core_words(void);
void compile_cell(cell_t cell);

#endif  // CORE_H