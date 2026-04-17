#pragma once

#include "defines.h"

typedef enum darray_meta_data
{
	DARRAY_ELEM_SIZE = 0,
	DARRAY_LENGTH = 1,
	DARRAY_CAPACITY = 2,
	DARRAY_HEAD = 3
} darray_meta_data;

#define darray_create(elem_type) _darray_initialize(sizeof(elem_type));

#define darray_create_size(elem_type, size) _darray_initialize_size(sizeof(elem_type), size)

#define darray_push_back(array, value)               \
	{                                                \
		typeof(value) temp = (value);                \
		(array) = _darray_push_back((array), &temp); \
	}

#define darray_capacity(darray) _darray_header_field_get(darray, DARRAY_CAPACITY)
#define darray_length(darray) _darray_header_field_get(darray, DARRAY_LENGTH)
#define darray_elem_size(darray) _darray_header_field_get(darray, DARRAY_ELEM_SIZE)

#define darray_header_field_set( \
	darray, header_field, value) _darray_header_field_set(darray, header_field, value)

#define darray_destroy(darray) _darray_destroy(darray)

#define darray_resize(darray, new_size) _darray_resize(darray, new_size)

void *_darray_initialize(u64 elem_size);

void *_darray_initialize_size(u64 elem_size, u64 initial_capacity);

void *_darray_push_back(void *darray, const void *elem);

void _darray_destroy(void *darray);

void* _darray_resize(void *darray, u64 new_size);

u64 _darray_header_field_get(void *darray, u64 field);

void _darray_header_field_set(void *darray, u64 field, u64 value);
