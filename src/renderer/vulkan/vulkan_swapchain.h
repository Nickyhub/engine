#pragma once
#include <vulkan/vulkan.h>
#include "defines.h"

#include "vulkan_device.h"
#include "vulkan_sync_objects.h"
#include "vulkan_renderpass.h"
#include "vulkan_framebuffer.h"

b8 vulkan_swapchain_create(
	u16 width,
	u16 height,
	vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_swapchain *out_swapchain);

b8 vulkan_swapchain_recreate(
	vulkan_swapchain *swapchain,
	u16 width,
	u16 height);

void vulkan_swapchain_destroy(vulkan_swapchain* swapchain);

b8 vulkan_swapchain_acquire_next_image(
	vulkan_swapchain* swapchain,
	vulkan_renderpass* renderpass,
	VkSemaphore semaphore);

b8 vulkan_swapchain_present(vulkan_swapchain *swapchain, VkSemaphore* semaphores);

// these are called externally from the renderer backend and are only used as an interface 
// to the swapchain, create and destroy do not interfere with the framebuffers
b8 vulkan_swapchain_create_framebuffers(vulkan_swapchain* swapchain, vulkan_renderpass* renderpass);
void vulkan_swapchain_destroy_framebuffers(vulkan_swapchain *swapchain, vulkan_renderpass *renderpass);
