#include "dictionary.h"

#include <ctype.h>
#include <string.h>

#include "debug.h"
#include "error.h"
#include "utf8.h"
#include "util.h"

// Dictionary storage
#define MAX_DICT_ENTRIES 500
static dictionary_entry_t dictionary[MAX_DICT_ENTRIES];
static int dict_size = 0;

// Dictionary management

void init_dictionary(void) {
  dict_size = 0;
  memset(dictionary, 0, sizeof(dictionary));
  debug("Dictionary initialized");
}

void add_cell(const char* name, cell_t def, const char* help) {
  check_dictionary();

  utf8_strncpy_safe(dictionary[dict_size].name, name, MAX_NAME_LENGTH - 1);
  dictionary[dict_size].name[MAX_NAME_LENGTH - 1] = '\0';

  dictionary[dict_size].definition = def;
  dictionary[dict_size].help = help;

  debug("Added '%s' to dictionary at index %d", name, dict_size);
  dict_size++;
}

void add_native_word(const char* name, native_func_t func, const char* help) {
  cell_t def = {0};
  def.type = CELL_NATIVE;
  def.payload.native = func;
  add_cell(name, def, help);
}

void check_dictionary(void) {
  if (dict_size >= MAX_DICT_ENTRIES) {
    error(&main_context, "Dictionary full");
  }
}

void add_native_word_immediate(const char* name, native_func_t func, const char* help) {
  cell_t def = {0};
  def.type = CELL_NATIVE;
  def.flags = CELL_FLAG_IMMEDIATE;  // Mark as immediate
  def.payload.native = func;
  add_cell(name, def, help);
}

dictionary_entry_t* find_word(const char* name) {
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

int get_dictionary_size(void) { return dict_size; }

dictionary_entry_t* get_dictionary_entry(int index) {
  if (index < 0 || index >= dict_size) {
    return NULL;
  }
  return &dictionary[index];
}

// Dictionary cleanup for testing
int save_dictionary_size(void) { return dict_size; }

void restore_dictionary_size(int saved_size) {
  if (saved_size < 0 || saved_size > dict_size) {
    return;  // Invalid size
  }

  // Release any allocated memory for words beyond saved_size
  for (int i = saved_size; i < dict_size; i++) {
    release(&dictionary[i].definition);
  }

  // Reset dictionary size
  dict_size = saved_size;
  debug("Dictionary restored to size %d", dict_size);
}