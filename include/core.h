#ifndef CORE_H
#define CORE_H

#include "cell.h"

extern bool compilation_mode;
extern cell_array_t* compiling_definition;
extern char compiling_word_name[];

// Add core language words to the dictionary
void add_core_words(void);
void compile_cell(cell_t cell);

#endif  // CORE_H