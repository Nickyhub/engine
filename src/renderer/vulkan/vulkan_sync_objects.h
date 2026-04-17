#pragma once

#include <vulkan/vulkan.h>

#include "vulkan_device.h"



b8 vulkan_semaphore_create(
	const vulkan_device* device,
	const VkAllocationCallbacks* allocator,
	vulkan_semaphore* out_semaphore);

void vulkan_semaphore_destroy(vulkan_semaphore* semaphore);

b8 vulkan_fence_create(
	const vulkan_device* device,
	const VkAllocationCallbacks* allocator,
	vulkan_fence* out_fence);

void vulkan_fence_destroy(vulkan_fence* fence);
