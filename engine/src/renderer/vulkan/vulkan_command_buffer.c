#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

#include "core/logger.h"

b8 vulkan_command_buffer_create(const vulkan_device *device,
								VkCommandPool pool,
								vulkan_command_buffer *out_command_buffer)
{
	out_command_buffer->command_pool = pool;
	out_command_buffer->device = device;

	VkCommandBufferAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	VkResult result = vkAllocateCommandBuffers(device->handle,
									  &alloc_info,
									  &out_command_buffer->handle);
	if(result != VK_SUCCESS) {
		return false;
	}

	out_command_buffer->state = COMMAND_BUFFER_CREATED;
	return true;
}

	vulkan_command_buffer_state state;
	VkCommandBuffer handle;
	const vulkan_device* device;
	VkCommandPool command_pool;

void vulkan_command_buffer_destroy(vulkan_command_buffer* buffer) {
	vkFreeCommandBuffers(buffer->device->handle, buffer->command_pool, 1, &buffer->handle);
	buffer->state = COMMAND_BUFFER_DEALLOCATED;
	buffer->device = 0;
	buffer->command_pool = 0;
}

b8 vulkan_command_buffer_begin(vulkan_command_buffer *command_buffer)
{
	VkCommandBufferBeginInfo begin_info= {0};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = 0;

	VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
	command_buffer->state = COMMAND_BUFFER_RECORDING;
	return true;
}

b8 vulkan_command_buffer_end(vulkan_command_buffer *command_buffer)
{
	// commandBuffer->s_State = 0;
	VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
	command_buffer->state = COMMAND_BUFFER_RECORDING_ENDED;
	return true;
}

vulkan_command_buffer vulkan_command_buffer_begin_single_use(const vulkan_device *device, VkCommandPool pool)
{
	VkCommandBufferAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = pool;
	alloc_info.commandBufferCount = 1;

	vulkan_command_buffer command_buffer = {0};
	vkAllocateCommandBuffers(device->handle, &alloc_info, &command_buffer.handle);

	VkCommandBufferBeginInfo begin_info = {0};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(command_buffer.handle, &begin_info));
	return command_buffer;
}

void vulkan_command_buffer_end_single_use(vulkan_command_buffer *command_buffer,
										  const VkQueue *queue,
										  const vulkan_device *device)
{
	vkEndCommandBuffer(command_buffer->handle);
	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer->handle;

	vkQueueSubmit(*queue,
				  1,
				  &submit_info,
				  VK_NULL_HANDLE);
	vkQueueWaitIdle(*queue);

	vkFreeCommandBuffers(device->handle,
						 device->command_pool,
						 1,
						 &command_buffer->handle);
	command_buffer->state = COMMAND_BUFFER_RECORDING_ENDED;
}
