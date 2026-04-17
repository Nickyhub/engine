#include <memory.h>
#include <stdlib.h>

#include "ememory.h"
#include "core/logger.h"
static memory_system_state *mem_state;

void memory_system_initialize()
{
    mem_state = malloc(sizeof(memory_system_state));
    ezero_out(mem_state, sizeof(memory_system_state));
    mem_state->allocated_memory[MEMORY_TYPE_SYSTEM_STATE] += sizeof(memory_system_state);
}

void memory_system_shutdown()
{
    print_memory_stats();
    efree(mem_state, sizeof(memory_system_state), MEMORY_TYPE_SYSTEM_STATE);
    mem_state = 0;
}

void *eallocate(u64 size, memory_allocation_type type)
{
    mem_state->allocated_memory[type] += size;
    return malloc(size);
}

void efree(void *ptr, u64 size, memory_allocation_type type)
{
    if (mem_state)
    {
        mem_state->allocated_memory[type] -= size;
    }
    free(ptr);
}

void ecopy(const void *src, void *dst, u32 size)
{
    memcpy(dst, src, size);
}

void ezero_out(void *dst, u32 size)
{
    memset(dst, 0, size);
}

void print_memory_stats()
{
    EN_DEBUG("MEMORY STATS:");
    EN_DEBUG("MEMORY_TYPE_DARRAY: %d", mem_state->allocated_memory[MEMORY_TYPE_DARRAY]);
    EN_DEBUG("MEMORY_TYPE_SYSTEM_STATE: %d", mem_state->allocated_memory[MEMORY_TYPE_SYSTEM_STATE]);
    EN_DEBUG("MEMORY_TYPE_VULKAN_RENDERER: %d", mem_state->allocated_memory[MEMORY_TYPE_VULKAN_RENDERER]);
}
