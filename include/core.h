#ifndef CORE_H
#define CORE_H

#include <stdbool.h>

#include "cell.h"

extern bool compilation_mode;
extern cell_array_t* compiling_definition;
extern char compiling_word_name[];

// Add core language words to the dictionary
void add_core_words(void);

#endif  // CORE_H