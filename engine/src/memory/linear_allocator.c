#include "linear_allocator.h"

#include "ememory.h"
#include "core/logger.h"

b8 linear_allocator_create(u64 size, linear_allocator *out_allocator)
{
    out_allocator->total_size = size;
    out_allocator->current_size = 0;
    out_allocator->memory_block = eallocate(size, MEMORY_TYPE_LINEAR_ALLOCATOR);
    if (!out_allocator->memory_block)
    {
        EN_ERROR("Failed to allocte %llu bytes for linear allocator creation.", size);
        return false;
    }
    return true;
}

void linear_allocator_destroy(linear_allocator *allocator)
{
    allocator->total_size = 0;
    allocator->current_size = 0;
    efree(allocator->memory_block, allocator->total_size, MEMORY_TYPE_LINEAR_ALLOCATOR);
}

void *linear_allocator_allocate(linear_allocator *allocator, u64 size)
{
    if(allocator->current_size + size > allocator->total_size) {
        EN_ERROR("Failed to allocte %llu bytes for linear allocator allocation. NULL is returned.", size);
        return 0;
    }

    void* block = (u8*)allocator->memory_block + allocator->current_size;
    allocator->current_size += size;
    return block;
}