#include "darray.h"
#include "memory/ememory.h"
#include "core/logger.h"

#define DARRAY_INITIAL_CAPACITY 10

void *_darray_initialize(u64 elem_size)
{
    return _darray_initialize_size(elem_size, DARRAY_INITIAL_CAPACITY);
}

void *_darray_initialize_size(u64 elem_size, u64 initial_capacity)
{
    u64 header_size = DARRAY_HEAD * sizeof(u64);
    u64 *darray = eallocate(header_size + elem_size * initial_capacity, MEMORY_TYPE_DARRAY);
    ezero_out(darray, header_size + elem_size * initial_capacity);

    darray[DARRAY_ELEM_SIZE] = elem_size;
    darray[DARRAY_LENGTH] = 0;
    darray[DARRAY_CAPACITY] = initial_capacity;

    return (void *)(darray + DARRAY_HEAD);
}

void* _darray_push_back(void *darray, const void *elem)
{
    u64 length = darray_length(darray);
    u64 capacity = darray_capacity(darray);
    u64 elem_size = darray_elem_size(darray);
    if (length < capacity)
    {
        ecopy(elem, (u8 *)darray + (length * elem_size), elem_size);
    }
    else
    {
        darray_resize(darray, capacity * 2);
        ecopy(elem,
             (u8 *)darray + (length * darray_elem_size(darray)),
             elem_size);

        EN_INFO("Freed old darray with capacity %d, capacity now is %d",
                capacity / 2,
                capacity);
    }
    darray_header_field_set(darray, DARRAY_LENGTH, darray_length(darray) + 1);
    return darray;
}

void _darray_destroy(void *darray)
{
    u64 *head = (u64 *)darray - DARRAY_HEAD;
    // ELEM_SIZE, LENGTH, CAPACITY, set to zero
    u64 capacity = darray_capacity(darray);
    u64 elem_size = darray_elem_size(darray);
    u64 header_size = DARRAY_HEAD * sizeof(u64);
    u64 total_size = elem_size * capacity + header_size;

    head[0] = 0;
    head[1] = 0;
    head[2] = 0;

    efree(head, total_size, MEMORY_TYPE_DARRAY);
}

void* _darray_resize(void *darray, u64 new_size)
{
    u64 length = darray_length(darray);
    u64 capacity = darray_capacity(darray);
    u64 elem_size = darray_elem_size(darray);

    if (new_size <= capacity)
    {
        EN_INFO("darray resize called with new_size smaller than capacity. Nothing happens");
        return (void*)0;
    }

    void* temp = _darray_initialize_size(elem_size, capacity * 2);
    _darray_header_field_set(temp, DARRAY_LENGTH, length);

    ecopy(darray, temp, length * elem_size);
    _darray_destroy(darray);
    return temp;
}

u64 _darray_header_field_get(void *darray, u64 field)
{
    u64 *head = (u64 *)darray - DARRAY_HEAD;
    return head[field];
}

void _darray_header_field_set(void *darray, u64 field, u64 value)
{
    u64 *head = (u64 *)darray - DARRAY_HEAD;
    head[field] = value;
}