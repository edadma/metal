#include "tools.h"

#include <stdio.h>
#include <stdlib.h>

#include "dictionary.h"
#include "metal.h"
#include "stack.h"

// External references to dictionary (defined in main.c)
extern dict_entry_t dictionary[];
extern int dict_size;

// Stack introspection

static void native_dot_s(context_t* ctx) { print_data_stack(ctx); }

// System control

static void native_bye([[maybe_unused]] context_t* ctx) {
  printf("Goodbye!\n");
  exit(0);
}

// Meta commands

static void native_words([[maybe_unused]] context_t* ctx) {
  printf("Dictionary (%d words):\n", dict_get_size());

  int words_per_line = 8;  // Adjust for readability
  for (int i = 0; i < dict_get_size(); i++) {
    printf("%-12s", dict_get_entry(i)->name);  // Left-aligned, 12 chars wide

    if ((i + 1) % words_per_line == 0 || i == dict_get_size() - 1) {
      printf("\n");
    }
  }
}

static void native_help([[maybe_unused]] context_t* ctx) {
  // We need to get the next token from input
  // For now, let's implement a simple version that shows all help
  printf("Available words with help:\n\n");

  for (int i = 0; i < dict_get_size(); i++) {
    dict_entry_t* entry = dict_get_entry(i);

    printf("%-12s %s\n", entry->name, entry->help);
  }
}

// Register all tool words
void add_tools_words(void) {
  // Stack introspection
  dict_add_native_word(".S", native_dot_s, "( -- ) Show stack contents");

  // System control
  dict_add_native_word("BYE", native_bye, "( -- ) Exit Metal");

  // Meta commands
  dict_add_native_word("WORDS", native_words,
                       "( -- ) List all available words");
  dict_add_native_word("HELP", native_help, "( -- ) Show help for all words");
}