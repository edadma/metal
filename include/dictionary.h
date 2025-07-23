#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "metal.h"

// Dictionary management
void dictionary_init(void);
void dict_add_native_word(const char* name, native_func_t func,
                          const char* help);
dictionary_entry_t* find_word(const char* name);

// Dictionary introspection (for tools)
int get_dictionary_size(void);
dictionary_entry_t* get_dictionary_entry(int index);

#endif  // DICTIONARY_H