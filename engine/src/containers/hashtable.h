#pragma once

#include "defines.h"

/** @brief Represents a simple hashtable. Members of this structure
 * should not be modified outside the function associated with it.
 *
 * For non-pointer types, table retains a copy of the value. For
 * pointer types, ake sure to use the _ptr setter and getter. Table
 * does not take ownership of pointers or associated memory allocations,
 * and should be managed externally.
 */

typedef struct hashtable
{
    u64 element_size;
    u32 element_count;
    b8 is_pointer_type;
    void *memory;
} hashtable;

void hashtable_create(u64 element_size, u32 element_count, void *memory, b8 is_pointer_type, hashtable *out_hashtable);

void hashtable_destroy(hashtable *table);

b8 hashtable_get(hashtable *table, const char *name, void *out_value);

b8 hashtable_set(hashtable *table, const char *name, void *value);

b8 hashtable_get_ptr(hashtable *table, const char *name, void **out_value);

b8 hashtable_set_ptr(hashtable *table, const char *name, void **value);

b8 hashtable_fill(hashtable* table, void* value);