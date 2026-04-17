#pragma once

#include <vulkan/vulkan.h>
#include "math/math_types.h"

#include "vulkan_device.h"

b8 vulkan_buffer_create(const vulkan_device*,
					 VkDeviceSize size, 
					 VkBufferUsageFlagBits usage,
					 VkMemoryPropertyFlags flags,
					 const VkAllocationCallbacks* allocator,
					 vulkan_buffer* out_buffer);

b8 vulkan_buffer_copy_buffer(vulkan_buffer* src_buffer,
						  vulkan_buffer* dst_buffer,
						  VkDeviceSize size,
						  VkQueue queue);

void vulkan_buffer_destroy(vulkan_buffer* buffer);

i32 vulkan_buffer_find_memory_type(const vulkan_device* device,
								   u32 type_filter,
								   VkMemoryPropertyFlags properties);
