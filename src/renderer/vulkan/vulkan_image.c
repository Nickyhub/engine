#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include <stb_image.h>

#include "vulkan_image.h"
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

#include "core/application.h"
#include "memory/ememory.h"

b8 create_image(
	u32 width,
	u32 height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags mem_flags,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_image *out_image);

b8 create_image_view(VkFormat format, VkImageAspectFlags aspect_flags, vulkan_image *image);

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
	vulkan_image *out_image)
{
	out_image->device = device;
	out_image->width = width;
	out_image->height = height;
	create_image(width, height, format, tiling, usage, mem_flags, device, allocator, out_image);
	if (create_view)
	{
		create_image_view(format, aspect_flags, out_image);
	}
	out_image->device = device;
	out_image->allocator = allocator;
}

void vulkan_image_destroy(vulkan_image *image)
{
	if (image->sampler != NULL)
	{
		vkDestroySampler(image->device->handle, image->sampler, image->allocator);
	}
	vkDestroyImageView(image->device->handle, image->view, image->allocator);
	vkDestroyImage(image->device->handle, image->handle, image->allocator);
	vkFreeMemory(image->device->handle, image->memory, image->allocator);
	EN_DEBUG("Vulkan image destroyed.");
}

b8 vulkan_image_has_stencil_component(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

/**
 * Depending on the usage a depth image and its view will be created. If it is a colored image it
 * will be loaded and created with a staging buffer.
 * TODO Do not hard code the path of colored image. Wrap this in a general texture system
 */

b8 create_image(
	u32 width,
	u32 height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags mem_flags,
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_image *out_image)
{
	// Create the vulkan image
	VkImageCreateInfo image_info = {0};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 4;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.flags = 0; // Optional

	VK_CHECK(vkCreateImage(device->handle, &image_info, allocator, &out_image->handle));
	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(device->handle, out_image->handle, &mem_requirements);

	VkMemoryAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = vulkan_buffer_find_memory_type(device, mem_requirements.memoryTypeBits, mem_flags);

	if (vkAllocateMemory(device->handle, &alloc_info, 0, &out_image->memory) != VK_SUCCESS)
	{
		EN_ERROR("Failed to allocate image memory.");
		return false;
	}

	vkBindImageMemory(device->handle, out_image->handle, out_image->memory, 0);
	return true;
}

b8 create_image_view(VkFormat format, VkImageAspectFlags aspect_flags, vulkan_image *image)
{
	VkImageViewCreateInfo view_info = {0};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image->handle;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(image->device->handle, &view_info, image->allocator, &image->view));
	return true;
}

b8 vulkan_image_create_texture_sampler(VkSampler *out_sampler, vulkan_image *image)
{
	VkSamplerCreateInfo sampler_info;
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;

	// Figure out max anisotropy
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(image->device->physical_device, &properties);
	sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;

	VK_CHECK(vkCreateSampler(image->device->handle, &sampler_info, image->allocator, out_sampler));
	return true;
}

VkFormat vulkan_image_find_supported_format(
	VkFormat *candidates,
	VkImageTiling tiling,
	VkFormatFeatureFlags features,
	vulkan_image *image)
{
	VkFormat format;
	for (unsigned int i = 0; i < darray_length(candidates); i++)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(image->device->physical_device, candidates[i], &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			format = candidates[i];
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			format = candidates[i];
		}
	}
	EN_WARN("Failed to find supported image format for depth image. NULL is returned.");
	return format;
}

void vulkan_image_transition_image_layout(
	vulkan_image *image,
	VkFormat format,
	VkImageLayout old_layout,
	VkImageLayout new_layout)
{

	vulkan_command_buffer command_buffer = vulkan_command_buffer_begin_single_use(image->device, image->device->command_pool);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image->handle;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		EN_ERROR("Unsupported image layout transition.");
	}

	if (new_layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(
		command_buffer.handle,
		source_stage, destination_stage,
		0,
		0, 0,
		0, 0,
		1, &barrier);
	vulkan_command_buffer_end_single_use(&command_buffer, &image->device->graphics_queue, image->device);
}

void vulkan_image_copy_buffer_to_image(vulkan_image *image, const VkBuffer *buffer, u32 width, u32 height)
{
	vulkan_command_buffer command_buffer = vulkan_command_buffer_begin_single_use(image->device, image->device->command_pool);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = (VkOffset3D){0, 0, 0};
	region.imageExtent = (VkExtent3D){
		width,
		height,
		1};

	vkCmdCopyBufferToImage(
		command_buffer.handle,
		*buffer,
		image->handle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);
	vulkan_command_buffer_end_single_use(&command_buffer, &image->device->graphics_queue, image->device);
}
