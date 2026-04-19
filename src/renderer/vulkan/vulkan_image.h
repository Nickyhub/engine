#pragma once

#include <vulkan/vulkan.h>

#include "vulkan_device.h"
#include "containers/darray.h"

b8 vulkan_image_create(
	u32 width,
	u32 height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkImageAspectFlags aspect_flags,
	VkMemoryPropertyFlags mem_flags,
	b8 create_view,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_image *out_image);

void vulkan_image_destroy(vulkan_image* image);

VkFormat vulkan_image_find_supported_format(
	VkFormat *candidates, // darray
	VkImageTiling tiling,
	VkFormatFeatureFlags features,
	vulkan_image *image);

b8 vulkan_image_has_stencil_component(VkFormat format);

b8 vulkan_image_create_texture_sampler(VkSampler* out_sampler, vulkan_image* image);

void vulkan_image_transition_image_layout(
	vulkan_image* image,
	VkFormat format,
	VkImageLayout old_layout,
	VkImageLayout new_layout
);

void vulkan_image_copy_buffer_to_image(vulkan_image* image, const VkBuffer* buffer, u32 width, u32 height);