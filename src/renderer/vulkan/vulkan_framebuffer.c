#include "vulkan_framebuffer.h"
#include "vulkan_utils.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"

#include "core/logger.h"

b8 vulkan_framebuffer_create(
	u32 width,
	u32 height,
	u32 image_index,
	const VkAllocationCallbacks *allocator,
	vulkan_device *device,
	const vulkan_image *depth_image,
	const vulkan_renderpass *renderpass,
	vulkan_swapchain *swapchain,
	vulkan_framebuffer *out_framebuffer)
{

	VkImageView attachments[2];
	attachments[0] = swapchain->image_views[image_index];
	attachments[1] = depth_image->view;

	VkFramebufferCreateInfo framebuffer_info = {0};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = renderpass->handle;
	framebuffer_info.attachmentCount = (uint32_t)2;
	framebuffer_info.pAttachments = attachments;
	framebuffer_info.width = width;
	framebuffer_info.height = height;
	framebuffer_info.layers = 1;

	VkResult result = vkCreateFramebuffer(device->handle,
										  &framebuffer_info,
										  allocator,
										  &out_framebuffer->handle);

	if (result != VK_SUCCESS)
	{
		return false;
	}

	out_framebuffer->device = device;
	out_framebuffer->allocator = allocator;
	return true;
}

void vulkan_framebuffer_destroy(vulkan_framebuffer *framebuffer)
{
	vkDestroyFramebuffer(framebuffer->device->handle, framebuffer->handle, framebuffer->allocator);
	framebuffer->device = 0;
	framebuffer->allocator = 0;
	EN_DEBUG("Vulkan framebuffer destroyed.");
}
