#pragma once

#include "defines.h"

typedef struct linear_allocator{
    u64 total_size;
    u64 current_size;
    void* memory_block;
} linear_allocator;

b8 linear_allocator_create(u64 size, linear_allocator* out_allocator);

void linear_allocator_destroy(linear_allocator* allocator);

void* linear_allocator_allocate(linear_allocator* alloator, u64 size);