#include "core/logger.h"

#include "vulkan_swapchain.h"
#include "vulkan_utils.h"

b8 create(
	u16 width,
	u16 height,
	vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_swapchain *out_swapchain);

void destroy(vulkan_swapchain *swapchain);

b8 vulkan_swapchain_create(
	u16 width,
	u16 height,
	vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_swapchain *out_swapchain)
{
	if (!create(width, height, device, allocator, out_swapchain))
	{
		return false;
	}
	return true;
}

b8 vulkan_swapchain_recreate(vulkan_swapchain *swapchain, u16 width, u16 height)
{
	vkDeviceWaitIdle(swapchain->device->handle);
	destroy(swapchain);
	if (!create(width, height, swapchain->device, swapchain->allocator, swapchain))
	{
		EN_ERROR("Failed to create swapchain while recreating the swapchain.");
		return false;
	}
	return true;
}

void vulkan_swapchain_destroy(vulkan_swapchain *swapchain)
{
	destroy(swapchain);
}

b8 create(
	u16 width,
	u16 height,
	vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	vulkan_swapchain *out_swapchain)
{
	out_swapchain->width = width;
	out_swapchain->height = height;
	out_swapchain->device = device;
	out_swapchain->allocator = allocator;

	// Choose the swapchain surface format
	b8 format_found = false;

	for (u32 i = 0; i < device->support_info.format_count; i++)
	{
		VkSurfaceFormatKHR f = device->support_info.formats[i];
		if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			out_swapchain->surface_format = f;
			EN_INFO("Swapchain format found.");
			format_found = true;
			break;
		}
	}
	// Choose the first format that is available if no suitable format has been found
	if (!format_found)
	{
		VkSurfaceFormatKHR f = device->support_info.formats[0];
		out_swapchain->surface_format = f;
		EN_WARN("Swapchain has chosen a format that is not optimal because the optimal format is not available.");
	}

	// Choose presentation mode
	// Set default
	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
	out_swapchain->present_mode = mode;
	// Look for better mode
	for (u32 i = 0; i < device->support_info.present_mode_count; i++)
	{
		if (device->support_info.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			out_swapchain->present_mode = device->support_info.present_modes[i];
		}
	}

	// Choose the swapchain extent
	// NOTE set up with surface capabilities as this custom way may lead to problems
	VkExtent2D swap_extent = {0};
	swap_extent.height = height;
	swap_extent.width = width;
	out_swapchain->height = height;
	out_swapchain->width = width;
	out_swapchain->extent = swap_extent;

	u32 image_count = device->support_info.capabilities.minImageCount + 1;
	if (device->support_info.capabilities.maxImageCount > 0 &&
		image_count > device->support_info.capabilities.maxImageCount)
	{
		image_count = device->support_info.capabilities.maxImageCount;
	}

	out_swapchain->max_frames_in_flight = image_count - 1;

	// Actually creating the swapchain
	VkSwapchainCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = device->surface->handle;

	create_info.minImageCount = image_count;
	create_info.imageFormat = out_swapchain->surface_format.format;
	create_info.imageColorSpace = out_swapchain->surface_format.colorSpace;
	create_info.imageExtent = swap_extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Determining the image sharing mode
	if (device->present_queue_family_index != device->graphics_queue_family_index)
	{
		u32 family_indices[] = {device->present_queue_family_index, device->graphics_queue_family_index};
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = family_indices;
		EN_TRACE("Vulkan swapchain imageSharing mode is concurrent.");
	}
	else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = 0;
		EN_TRACE("Vulkan swapchain imageSharing mode is exclusive.");
	}
	create_info.preTransform = device->support_info.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = out_swapchain->present_mode;
	create_info.clipped = VK_TRUE;
	// TODO when resizing provide the old swapchain
	create_info.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(device->handle, &create_info, allocator, &out_swapchain->handle));
	EN_INFO("Swapchain created with width/height: %d/%d", width, height);

	// Retrieving the swapchain image handles to render to them later
	vkGetSwapchainImagesKHR(device->handle, out_swapchain->handle, (uint32_t *)&out_swapchain->image_count, 0);
	out_swapchain->images = darray_create_size(VkImage, image_count);
	vkGetSwapchainImagesKHR(
		device->handle,
		out_swapchain->handle,
		(uint32_t *)&out_swapchain->image_count,
		out_swapchain->images);

	// Create image views
	out_swapchain->image_views = darray_create_size(VkImageView, image_count);
	out_swapchain->image_view_count = image_count;
	for (u32 i = 0; i < image_count; i++)
	{
		// TODO call VulkanImageUtils::CreateImageView. Somehow this should be cleaned up anyway then.
		VkImageViewCreateInfo create_info = {0};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = out_swapchain->images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = out_swapchain->surface_format.format;

		// Swizzle color channels (maybe create red texture. Test around with this. For now set to default
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Specify the images purpose. IN this case the are color targets without mipmapping or layers
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(device->handle,
								   &create_info,
								   allocator,
								   &out_swapchain->image_views[i]));
	}

	// Create depth image
	vulkan_device_find_depth_format(device);
	vulkan_image_create(
		width,
		height,
		device->depth_format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		true,
		device,
		allocator,
		&out_swapchain->depth_image);

	return true;
}

b8 vulkan_swapchain_acquire_next_image(
	vulkan_swapchain *swapchain,
	vulkan_renderpass *renderpass,
	VkSemaphore semaphore)
{
	VkResult result = vkAcquireNextImageKHR(
		swapchain->device->handle,
		swapchain->handle,
		UINT64_MAX,
		semaphore,
		VK_NULL_HANDLE,
		&swapchain->current_swapchain_image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		if (!vulkan_swapchain_recreate(
				swapchain, swapchain->width,
				swapchain->height))
		{
			EN_ERROR("Failed to recreate Swapchain.");
			return false;
		}
		return false;
	}
	return true;
}

b8 vulkan_swapchain_present(vulkan_swapchain *swapchain, VkSemaphore *semaphores)
{
	VkPresentInfoKHR present_info = {0};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = semaphores;

	VkSwapchainKHR swapchains[] = {swapchain->handle};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = (u32 *)&swapchain->current_swapchain_image_index;

	vkQueuePresentKHR(swapchain->device->present_queue, &present_info);
	return true;
}

b8 vulkan_swapchain_create_framebuffers(vulkan_swapchain *swapchain, vulkan_renderpass *renderpass)
{
	swapchain->framebuffers = darray_create_size(vulkan_framebuffer, swapchain->image_count);
	for (u32 i = 0; i < swapchain->image_count; i++)
	{
		if (!vulkan_framebuffer_create(
				swapchain->width,
				swapchain->height,
				i,
				swapchain->allocator,
				swapchain->device,
				&swapchain->depth_image,
				renderpass,
				swapchain,
				&swapchain->framebuffers[i]))
		{
			EN_ERROR("Failed to create vulkan framebuffer in swapchain.");
			return false;
		}
	}
	return true;
}

void vulkan_swapchain_destroy_framebuffers(vulkan_swapchain *swapchain, vulkan_renderpass *renderpass)
{
	for (u32 i = 0; i < swapchain->image_count; i++)
	{
		vulkan_framebuffer_destroy(&swapchain->framebuffers[i]);
	}
	darray_destroy(swapchain->framebuffers);
}

void destroy(vulkan_swapchain *swapchain)
{
	// Destroy image views before destroying the swapchain itself
	for (u32 i = 0; i < swapchain->image_view_count; i++)
	{
		vkDestroyImageView(swapchain->device->handle, swapchain->image_views[i], swapchain->allocator);
	}

	vulkan_image_destroy(&swapchain->depth_image);

	darray_destroy(swapchain->images);
	darray_destroy(swapchain->image_views);

	swapchain->height = 0;
	swapchain->width = 0;
	vkDestroySwapchainKHR(swapchain->device->handle, swapchain->handle, swapchain->allocator);
	swapchain->handle = 0;
}
