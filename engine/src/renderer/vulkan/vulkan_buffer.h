#pragma once

#include <vulkan/vulkan.h>
#include "math/math_types.h"

#include "vulkan_device.h"

b8 vulkan_buffer_create(
	VkDeviceSize size,
	VkBufferUsageFlagBits usage,
	VkMemoryPropertyFlags mem_flags,
	b8 bind_on_create,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_buffer *out_buffer);

void *vulkan_buffer_lock_memory(
	vulkan_buffer *buffer,
	u64 offset,
	u64 size,
	u32 flags,
	const vulkan_device *device);

void vulkan_buffer_unlock_memory(
	const vulkan_device *device,
	vulkan_buffer *buffer);

void vulkan_buffer_load_data(
	vulkan_buffer *buffer,
	u64 offset,
	u64 size,
	u32 flags,
	const void *data,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator);

void vulkan_buffer_bind(vulkan_buffer *buffer, u64 offset);

b8 vulkan_buffer_resize(
	u64 new_size,
	vulkan_buffer *buffer,
	VkQueue queue,
	VkCommandPool pool,
	const vulkan_device *device,
	const VkAllocationCallbacks* allocator);

b8 vulkan_buffer_copy_buffer(vulkan_buffer *src_buffer,
							 vulkan_buffer *dst_buffer,
							 VkDeviceSize size,
							 VkQueue queue);

void vulkan_buffer_destroy(vulkan_buffer *buffer);

i32 vulkan_buffer_find_memory_type(const vulkan_device *device,
								   u32 type_filter,
								   VkMemoryPropertyFlags properties);
