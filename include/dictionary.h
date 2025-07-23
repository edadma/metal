#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "metal.h"

// Dictionary management
void dict_init(void);
void dict_add_native_word(const char* name, native_func_t func,
                          const char* help);
dict_entry_t* dict_find_word(const char* name);

// Dictionary introspection (for tools)
int dict_get_size(void);
dict_entry_t* dict_get_entry(int index);

#endif  // DICTIONARY_H