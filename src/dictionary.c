#include "dictionary.h"

#include <ctype.h>
#include <string.h>

#include "debug.h"
#include "util.h"

// Dictionary storage
#define MAX_DICT_ENTRIES 256
static dict_entry_t dictionary[MAX_DICT_ENTRIES];
static int dict_size = 0;

// Dictionary management

void dict_init(void) {
  dict_size = 0;
  memset(dictionary, 0, sizeof(dictionary));
  debug("Dictionary initialized");
}

void dict_add_native_word(const char* name, native_func_t func,
                          const char* help) {
  if (dict_size >= MAX_DICT_ENTRIES) {
    error("Dictionary full");
    return;
  }

  strncpy(dictionary[dict_size].name, name, 31);
  dictionary[dict_size].name[31] = '\0';

  cell_t def = {0};
  def.type = CELL_NATIVE;
  def.payload.native = func;

  dictionary[dict_size].definition = def;
  dictionary[dict_size].help = help;

  debug("Added word '%s' to dictionary at index %d", name, dict_size);
  dict_size++;
}

dict_entry_t* dict_find_word(const char* name) {
  for (int i = dict_size - 1; i >= 0; i--) {
    if (stricmp(dictionary[i].name, name) == 0) {
      debug("Found word '%s' at dictionary index %d", name, i);
      return &dictionary[i];
    }
  }
  debug("Word '%s' not found in dictionary", name);
  return NULL;
}

// Dictionary introspection

int dict_get_size(void) { return dict_size; }

dict_entry_t* dict_get_entry(int index) {
  if (index < 0 || index >= dict_size) {
    return NULL;
  }
  return &dictionary[index];
}