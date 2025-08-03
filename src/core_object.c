#include "array.h"
#include "dictionary.h"
#include "error.h"
#include "object.h"
#include "require.h"
#include "stack.h"
#include "strings.h"

static void native_empty_object(context_t* ctx) { data_push(ctx, new_empty_object(ctx)); }

// PUT ( object key value -- object ) Add/update key-value pair
static void native_put(context_t* ctx) {
  require_params(ctx, 3, "PUT");

  cell_t value_cell = data_pop_cell(ctx);
  cell_t key_cell = data_pop_cell(ctx);
  cell_t* object_cell = data_peek(ctx, 0);  // Leave object on stack

  if (object_cell->type != CELL_OBJECT) {
    error(ctx, "PUT: first argument must be object");
  }

  if (key_cell.type != CELL_STRING) {
    error(ctx, "PUT: key must be string");
  }

  object_t* obj = object_cell->payload.object;
  if (!obj) {
    error(ctx, "PUT: cannot modify empty object");
  }

  // Use object_set which handles key interning
  object_set(ctx, obj, &key_cell, &value_cell);

  release(&key_cell);
  release(&value_cell);
}

// HAS? ( object key -- object bool ) Check if key exists
static void native_has_q(context_t* ctx) {
  require_params(ctx, 2, "HAS?");

  cell_t key_cell = data_pop_cell(ctx);
  cell_t* object_cell = data_peek(ctx, 0);  // Leave object on stack

  if (object_cell->type != CELL_OBJECT) {
    error(ctx, "HAS?: first argument must be object");
  }

  if (key_cell.type != CELL_STRING) {
    error(ctx, "HAS?: key must be string");
  }

  bool has_key = false;
  object_t* obj = object_cell->payload.object;

  if (obj) {
    cell_t* found = object_get(ctx, obj, &key_cell);
    has_key = (found != NULL);
  }

  data_push(ctx, new_boolean(has_key));
  release(&key_cell);
}

// KEYS ( object -- object keys-array ) Get array of all keys
static void native_keys(context_t* ctx) {
  require_params(ctx, 1, "KEYS");

  cell_t* object_cell = data_peek(ctx, 0);  // Leave object on stack

  if (object_cell->type != CELL_OBJECT) {
    error(ctx, "KEYS: argument must be object");
  }

  object_t* obj = object_cell->payload.object;
  cell_t keys_array = new_empty_array(ctx);

  if (obj) {
    // Resize array to hold all keys
    array_t* keys_data = keys_array.payload.array;
    if (obj->length > 0) {
      resize_array_data(ctx, keys_data, obj->length);
      keys_data->length = obj->length;

      // Copy all keys as interned string cells
      for (size_t i = 0; i < obj->length; i++) {
        keys_data->elements[i] = new_interned_string(obj->pairs[i].key);
        retain(&keys_data->elements[i]);
      }
    }
  }

  data_push(ctx, keys_array);
}

// VALUES ( object -- object values-array ) Get array of all values
static void native_values(context_t* ctx) {
  require_params(ctx, 1, "VALUES");

  cell_t* object_cell = data_peek(ctx, 0);  // Leave object on stack

  if (object_cell->type != CELL_OBJECT) {
    error(ctx, "VALUES: argument must be object");
  }

  object_t* obj = object_cell->payload.object;
  cell_t values_array = new_empty_array(ctx);

  if (obj) {
    // Resize array to hold all values
    array_t* values_data = values_array.payload.array;
    if (obj->length > 0) {
      resize_array_data(ctx, values_data, obj->length);
      values_data->length = obj->length;

      // Copy all values
      for (size_t i = 0; i < obj->length; i++) {
        values_data->elements[i] = obj->pairs[i].value;
        retain(&values_data->elements[i]);
      }
    }
  }

  data_push(ctx, values_array);
}

// ENTRIES ( object -- object entries-array ) Get array of [key value] pairs
static void native_entries(context_t* ctx) {
  require_params(ctx, 1, "ENTRIES");

  cell_t* object_cell = data_peek(ctx, 0);  // Leave object on stack

  if (object_cell->type != CELL_OBJECT) {
    error(ctx, "ENTRIES: argument must be object");
  }

  object_t* obj = object_cell->payload.object;
  cell_t entries_array = new_empty_array(ctx);

  if (obj) {
    // Resize array to hold all entry pairs
    array_t* entries_data = entries_array.payload.array;
    if (obj->length > 0) {
      resize_array_data(ctx, entries_data, obj->length);
      entries_data->length = obj->length;

      // Create [key value] pair arrays
      for (size_t i = 0; i < obj->length; i++) {
        cell_t pair_array = new_empty_array(ctx);
        array_t* pair_data = pair_array.payload.array;

        // Resize to hold exactly 2 elements [key, value]
        resize_array_data(ctx, pair_data, 2);
        pair_data->length = 2;

        // Set key and value
        pair_data->elements[0] = new_interned_string(obj->pairs[i].key);
        pair_data->elements[1] = obj->pairs[i].value;
        retain(&pair_data->elements[0]);
        retain(&pair_data->elements[1]);

        entries_data->elements[i] = pair_array;
        retain(&entries_data->elements[i]);
      }
    }
  }

  data_push(ctx, entries_array);
}

// FROM-ENTRIES ( entries-array -- object ) Create object from [key value] pairs
static void native_from_entries(context_t* ctx) {
  require_params(ctx, 1, "FROM-ENTRIES");

  cell_t entries_cell = data_pop_cell(ctx);

  if (entries_cell.type != CELL_ARRAY) {
    error(ctx, "FROM-ENTRIES: argument must be array");
  }

  array_t* entries_data = entries_cell.payload.array;
  cell_t result_object = new_empty_object(ctx);
  object_t* result_obj = result_object.payload.object;

  if (entries_data) {
    // Process each entry pair
    for (size_t i = 0; i < entries_data->length; i++) {
      cell_t* entry = &entries_data->elements[i];

      if (entry->type != CELL_ARRAY) {
        error(ctx, "FROM-ENTRIES: each entry must be array");
      }

      array_t* pair_data = entry->payload.array;
      if (!pair_data || pair_data->length != 2) {
        error(ctx, "FROM-ENTRIES: each entry must be [key value] pair");
      }

      cell_t* key = &pair_data->elements[0];
      cell_t* value = &pair_data->elements[1];

      if (key->type != CELL_STRING) {
        error(ctx, "FROM-ENTRIES: entry key must be string");
      }

      // Use object_set to add the key-value pair
      object_set(ctx, result_obj, key, value);
    }
  }

  data_push(ctx, result_object);
  release(&entries_cell);
}

// Register all object words
void add_core_object_words(void) {
  // Object creation and manipulation
  add_native_word("{}", native_empty_object, "( -- object ) Create empty object");
  add_native_word("PUT", native_put, "( object key value -- object ) Add/update key-value pair");
  add_native_word("HAS?", native_has_q, "( object key -- object bool ) Check if key exists");

  // Object iteration and conversion
  add_native_word("KEYS", native_keys, "( object -- object keys-array ) Get array of all keys");
  add_native_word("VALUES", native_values, "( object -- object values-array ) Get array of all values");
  add_native_word("ENTRIES", native_entries, "( object -- object entries-array ) Get array of [key value] pairs");
  add_native_word("FROM-ENTRIES", native_from_entries, "( entries-array -- object ) Create object from [key value] pairs");
}
