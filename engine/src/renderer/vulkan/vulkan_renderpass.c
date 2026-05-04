#include "core/logger.h"

#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_utils.h"

b8 vulkan_renderpass_create(const vulkan_device *device, VkFormat color_format, const VkAllocationCallbacks *allocator, vulkan_renderpass *out_renderpass)
{
	out_renderpass->device = device;
	out_renderpass->allocator = allocator;

	VkAttachmentDescription color_attachment = {0};
	color_attachment.format = color_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref;
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {0};
	depth_attachment.format = device->depth_format; // set in the swapchain creation
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {0};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Group attachments to array
	VkAttachmentDescription attachments[2];
	attachments[0] = color_attachment;
	attachments[1] = depth_attachment;

	VkRenderPassCreateInfo render_pass_info = {0};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(device->handle, &render_pass_info, allocator, &out_renderpass->handle);
	if (result != VK_SUCCESS)
	{
		EN_ERROR("Failed to create renderpass.");
		return false;
	}

	EN_DEBUG("Renderpass created.");
	return true;
}

void vulkan_renderpass_destroy(vulkan_renderpass *renderpass)
{
	vkDestroyRenderPass(renderpass->device->handle, renderpass->handle, renderpass->allocator);
	EN_DEBUG("Vulkan renderpass destroyed.");
}

b8 vulkan_renderpass_begin(
	vulkan_command_buffer *command_buffer,
	VkExtent2D extent,
	vulkan_framebuffer *framebuffer,
	vulkan_renderpass *renderpass)
{
	VkRenderPassBeginInfo renderpass_info = {0};
	renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_info.renderPass = renderpass->handle;
	renderpass_info.framebuffer = framebuffer->handle;
	renderpass_info.renderArea.offset.x = 0;
	renderpass_info.renderArea.offset.y = 0;
	renderpass_info.renderArea.extent = extent;

	VkClearValue clear_values[2];

	clear_values[0].color = (VkClearColorValue){{0.0f, 0.1f, 0.1f, 1.0f}};
	clear_values[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

	renderpass_info.clearValueCount = 2;
	renderpass_info.pClearValues = clear_values;

	vkCmdBeginRenderPass(command_buffer->handle, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
	return true;
}

b8 vulkan_renderpass_end(vulkan_command_buffer *command_buffer)
{
	vkCmdEndRenderPass(command_buffer->handle);
	return true;
}
