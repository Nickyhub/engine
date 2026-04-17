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
	const vulkan_device* device,
	VkDeviceSize size,
	VkBufferUsageFlagBits usage,
	VkMemoryPropertyFlags flags,
	const VkAllocationCallbacks* allocator,
	vulkan_buffer* out_buffer) {
	
	out_buffer->device = device;
	out_buffer->size = size;
	out_buffer->allocator = allocator;

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

	VkMemoryAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_req.size;
	alloc_info.memoryTypeIndex = vulkan_buffer_find_memory_type(device,
																mem_req.memoryTypeBits,
																flags);

	// Allocate and bind the buffer memory
	VK_CHECK(vkAllocateMemory(device->handle,
		&alloc_info,
		out_buffer->allocator,
		&out_buffer->memory));
	//EN_INFO("Allocated vulkan memory with address: %p.", &m_Memory);
	vkBindBufferMemory(device->handle,
		out_buffer->handle,
		out_buffer->memory,
		0);
	return true;
}

b8 vulkan_buffer_copy_buffer(vulkan_buffer* src_buffer, vulkan_buffer* dst_buffer, VkDeviceSize size, VkQueue queue) {
	vulkan_command_buffer command_buffer = vulkan_command_buffer_begin_single_use(src_buffer->device, src_buffer->device->command_pool);

	VkBufferCopy copy_region;
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer.handle, src_buffer->handle, dst_buffer->handle, 1, &copy_region);

	vulkan_command_buffer_end_single_use(&command_buffer, &queue, src_buffer->device);
	return true;
}

i32 vulkan_buffer_find_memory_type(const vulkan_device* device, u32 typeFilter, VkMemoryPropertyFlags properties) {
	// Find the proper memory type
	VkPhysicalDeviceMemoryProperties mem_props = {0};
	vkGetPhysicalDeviceMemoryProperties(device->physical_device, &mem_props);
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & (properties))) {
			return i;
		}
	}
	EN_WARN("No appropriate memory type has been found. Returning 0.");
	return 0;
}

void vulkan_buffer_destroy(vulkan_buffer* buffer) {
	vkDestroyBuffer(buffer->device->handle,
		buffer->handle,
		buffer->allocator);
	vkFreeMemory(buffer->device->handle,
		buffer->memory,
		buffer->allocator);
	//EN_INFO("Freed vulkan memory with address: %p.", &m_Memory);
}