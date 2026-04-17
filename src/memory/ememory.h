#pragma once

#include "defines.h"

typedef enum memory_allocation_type {
    MEMORY_TYPE_DARRAY,
    MEMORY_TYPE_SYSTEM_STATE,
    MEMORY_TYPE_VULKAN_RENDERER,

    MEMORY_TYPE_UNDEFINED,
} memory_allocation_type;

typedef struct memory_system_state {
    u64 allocated_memory[MEMORY_TYPE_UNDEFINED];
} memory_system_state;

void memory_system_initialize();

void memory_system_shutdown();

//TODO:  - Implement memory allocators that use different means of getting memory
void* eallocate(u64 size, memory_allocation_type type);

void efree(void* ptr, u64 size, memory_allocation_type type);

void ecopy(const void* src, void* dst, u32 size);

void ezero_out(void* dst, u32 size);

void print_memory_stats();