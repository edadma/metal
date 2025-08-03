#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>

#include "cell.h"

// Object data management
object_t* create_object_data(context_t* ctx, size_t initial_capacity);
void resize_object_data(context_t* ctx, object_t* obj, size_t new_capacity);
void free_object_data(object_t* obj);

// Object operations
const string_t* get_string_from_cell(const cell_t* cell);
cell_t* object_get(context_t* ctx, const object_t* obj, const cell_t* key_cell);
void object_set(context_t* ctx, object_t* obj, const cell_t* key_cell, const cell_t* value_cell);

#endif  // OBJECT_H
