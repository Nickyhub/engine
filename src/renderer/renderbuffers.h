#pragma once

#include "renderer_types.inl"


b8 vertex_buffer_create(vertex_3d* vertices,
						const vulkan_device* device,
						const VkAllocationCallbacks* allocator,
						vertex_buffer* out_vertex_buffer);

void vertex_buffer_destroy(vertex_buffer* vertex_buffer);

b8 index_buffer_create(u32* indices, 
                       const vulkan_device* device,
                       const VkAllocationCallbacks* allocator,
                       index_buffer* out_index_buffer);

void index_buffer_destroy(index_buffer* index_buffer);

// TODO: uniform buffers?