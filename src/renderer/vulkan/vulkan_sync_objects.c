#include "vulkan_sync_objects.h"
#include "vulkan_utils.h"
#include "vulkan_device.h"

#include "core/logger.h"

b8 vulkan_semaphore_create(
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_semaphore *out_semaphore)
{
	out_semaphore->device = device;
	out_semaphore->allocator = allocator;
	VkSemaphoreCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VK_CHECK(vkCreateSemaphore(device->handle, &create_info, allocator, &out_semaphore->handle));
	return true;
}

void vulkan_semaphore_destroy(vulkan_semaphore *semaphore)
{
	vkDestroySemaphore(semaphore->device->handle, semaphore->handle, semaphore->allocator);
}

b8 vulkan_fence_create(
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_fence *out_fence)
{
	out_fence->device = device;
	out_fence->allocator = allocator;
	VkFenceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK_CHECK(vkCreateFence(device->handle, &create_info, allocator, &out_fence->handle));
	return true;
}

void vulkan_fence_destroy(vulkan_fence* fence)
{
	vkDestroyFence(fence->device->handle, fence->handle, fence->allocator);
}
