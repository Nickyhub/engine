#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_buffer.h"

b8 vulkan_pipeline_create(
	const vulkan_context* context,
	const vulkan_renderpass *renderpass,
	u32 attribute_count,
	VkVertexInputAttributeDescription *attributes, // darray
	u32 stage_count,
	VkPipelineShaderStageCreateInfo *shader_stages, // darray
	u32 descriptor_set_layout_count,
	VkDescriptorSetLayout* descriptor_set_layouts,	   // darray
	VkViewport viewport,
	VkRect2D scissor,
	b8 is_wireframe,
	vulkan_pipeline *out_pipeline);

void vulkan_pipeline_destroy(vulkan_pipeline *pipeline);

b8 vulkan_pipeline_create_descriptor_pool(vulkan_pipeline *pipeline);

void vulkan_pipeline_bind(vulkan_command_buffer* command_buffer, vulkan_pipeline* pipeline);

b8 vulkan_pipeline_create_descriptor_sets(
	const VkImageView *view,
	//const uniform_buffer *ub,
	const VkSampler *sampler,
	vulkan_pipeline *pipeline);

b8 vulkan_pipeline_create_descriptor_set_layout(vulkan_pipeline *pipeline);
