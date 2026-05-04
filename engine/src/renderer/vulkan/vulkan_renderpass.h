#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"

b8 vulkan_renderpass_create(const vulkan_device* device, VkFormat color_format, const VkAllocationCallbacks* allocator, vulkan_renderpass* out_renderpass);

void vulkan_renderpass_destroy(vulkan_renderpass* renderpass);

b8 vulkan_renderpass_begin(
	vulkan_command_buffer *command_buffer,
	VkExtent2D extent,
	vulkan_framebuffer *framebuffer,
	vulkan_renderpass *renderpass);

b8 vulkan_renderpass_end(vulkan_command_buffer* command_buffer);
