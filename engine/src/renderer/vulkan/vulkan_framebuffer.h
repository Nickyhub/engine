#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_renderpass.h"
#include "vulkan_image.h"


b8 vulkan_framebuffer_create(
	u32 width,
	u32 height,
	u32 image_index,
	const VkAllocationCallbacks* allocator,
	vulkan_device* device,
	const vulkan_image* depth_image,
	const vulkan_renderpass* renderpass,
	vulkan_swapchain* swapchain,
	vulkan_framebuffer* out_framebuffer
);

void vulkan_framebuffer_destroy(vulkan_framebuffer* framebuffer);
