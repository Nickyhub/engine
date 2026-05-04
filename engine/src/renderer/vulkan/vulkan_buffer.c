#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

#include "core/random.h"
#include "core/logger.h"
#include "memory/ememory.h"
#include "math/math_types.h"

b8 vulkan_buffer_create(
	VkDeviceSize size,
	VkBufferUsageFlagBits usage,
	VkMemoryPropertyFlags mem_flags,
	b8 bind_on_create,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_buffer *out_buffer)
{
	ezero_out(out_buffer, sizeof(vulkan_buffer));
	out_buffer->device = device;
	out_buffer->size = size;
	out_buffer->allocator = allocator;
	out_buffer->usage = usage;
	out_buffer->memory_property_flags = mem_flags;

	VkBufferCreateInfo buffer_info = {0};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(device->handle,
							&buffer_info,
							out_buffer->allocator,
							&out_buffer->handle));

	// Buffer is created but it needs actual memory associated with it
	VkMemoryRequirements mem_req = {0};
	vkGetBufferMemoryRequirements(device->handle, out_buffer->handle, &mem_req);

	out_buffer->memory_index = vulkan_buffer_find_memory_type(device,
															  mem_req.memoryTypeBits,
															  mem_flags);
	if (out_buffer->memory_index == -1)
	{
		EN_ERROR("Unable to find create vulkan buffer because the required memory type index was not found.");
		return false;
	}

	VkMemoryAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_req.size;
	alloc_info.memoryTypeIndex = out_buffer->memory_index;

		// Allocate and bind the buffer memory
		VK_CHECK(vkAllocateMemory(device->handle,
								  &alloc_info,
								  out_buffer->allocator,
								  &out_buffer->memory));

	EN_INFO("Allocated vulkan memory with size: %lu", size);
	if (bind_on_create)
	{
		vulkan_buffer_bind(out_buffer, 0);
	}
	return true;
}

void vulkan_buffer_bind(vulkan_buffer *buffer, u64 offset)
{
	vkBindBufferMemory(buffer->device->handle, buffer->handle, buffer->memory, offset);
}

b8 vulkan_buffer_copy_buffer(vulkan_buffer *src_buffer, vulkan_buffer *dst_buffer, VkDeviceSize size, VkQueue queue)
{
	vulkan_command_buffer command_buffer = vulkan_command_buffer_begin_single_use(src_buffer->device, src_buffer->device->command_pool);

	VkBufferCopy copy_region = {0};
	copy_region.size = size;
	// TODO: When needed do as function parameters
	copy_region.dstOffset = 0;
	copy_region.srcOffset = 0;
	vkCmdCopyBuffer(command_buffer.handle, src_buffer->handle, dst_buffer->handle, 1, &copy_region);

	vulkan_command_buffer_end_single_use(&command_buffer, &queue, src_buffer->device);
	return true;
}

i32 vulkan_buffer_find_memory_type(const vulkan_device *device, u32 typeFilter, VkMemoryPropertyFlags properties)
{
	// Find the proper memory type
	VkPhysicalDeviceMemoryProperties mem_props = {0};
	vkGetPhysicalDeviceMemoryProperties(device->physical_device, &mem_props);
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) &&
    	(mem_props.memoryTypes[i].propertyFlags & properties) == properties)		{
			return i;
		}
	}
	EN_WARN("No appropriate memory type has been found. Returning 0.");
	return -1;
}

void vulkan_buffer_destroy(vulkan_buffer *buffer)
{
	vkDestroyBuffer(buffer->device->handle,
					buffer->handle,
					buffer->allocator);
	vkFreeMemory(buffer->device->handle,
				 buffer->memory,
				 buffer->allocator);
	ezero_out(buffer, sizeof(vulkan_buffer));
}

b8 vulkan_buffer_resize(
	u64 new_size,
	vulkan_buffer *buffer,
	VkQueue queue,
	VkCommandPool pool,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator)
{
	if (new_size < buffer->size)
	{
		EN_WARN("Called vulkan_buffer_resize with smaller new_size than passed buffer. Nothing happens");
		return false;
	}

	vulkan_buffer new_buffer = {0};
	if (!vulkan_buffer_create(new_size, buffer->usage, buffer->memory_property_flags, true, device, allocator, &new_buffer))
	{
		EN_ERROR("vulkan_buffer_resize - Failed to create new buffer with size %lu", new_size);
		return false;
	}

	vulkan_buffer_copy_buffer(buffer, &new_buffer, new_size, queue);
	vkDeviceWaitIdle(device->handle);

	vkDestroyBuffer(buffer->device->handle,
					buffer->handle,
					buffer->allocator);
	vkFreeMemory(buffer->device->handle,
				 buffer->memory,
				 buffer->allocator);

	buffer->size = new_size;
	buffer->memory = new_buffer.memory;
	buffer->handle = new_buffer.handle;
	return true;
}

void *vulkan_buffer_lock_memory(
	vulkan_buffer *buffer,
	u64 offset,
	u64 size,
	u32 flags,
	const vulkan_device *device)
{
	void *data;
	vkMapMemory(device->handle, buffer->memory, offset, size, flags, &data);
	return data;
}

void vulkan_buffer_unlock_memory(
	const vulkan_device *device,
	vulkan_buffer *buffer)
{
	vkUnmapMemory(device->handle, buffer->memory);
}

void vulkan_buffer_load_data(
	vulkan_buffer *buffer,
	u64 offset,
	u64 size,
	u32 flags,
	const void *data,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator)
{
	void *data_ptr;
	VK_CHECK(vkMapMemory(device->handle, buffer->memory, offset, size, flags, &data_ptr));
	ecopy(data, data_ptr, size);
	vkUnmapMemory(device->handle, buffer->memory);
}