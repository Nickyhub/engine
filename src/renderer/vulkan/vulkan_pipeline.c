#include "vulkan_pipeline.h"
#include "vulkan_utils.h"
#include "vulkan_buffer.h"
#include "vulkan_framebuffer.h"

#include "containers/darray.h"
#include "core/Logger.h"
#include "memory/ememory.h"

b8 vulkan_pipeline_create(
	u32 width,
	u32 height,
	const vulkan_renderpass *renderpass,
	u32 attribute_count,
	VkVertexInputAttributeDescription *attributes, // darray
	u32 stage_count,
	VkPipelineShaderStageCreateInfo *shader_stages,	   // darray
	const vulkan_device *device,
	const VkAllocationCallbacks *allocator,
	b8 is_wireframe,
	vulkan_pipeline *out_pipeline)
{
	out_pipeline->device = device;
	// out_pipeline->frames_in_flight = frames_in_flight;

	// First create descriptor set layout
	vulkan_pipeline_create_descriptor_set_layout(out_pipeline);

	// Viewport
	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor = {0};
	scissor.offset = (VkOffset2D){0, 0};
	scissor.extent = (VkExtent2D){width, height};

	VkPipelineViewportStateCreateInfo viewport_state_info = {0};
	viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_info.viewportCount = 1;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pViewports = &viewport;
	viewport_state_info.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	// Maybe support wireframe rendering?
	rasterizer.polygonMode = is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = /*VK_CULL_MODE_BACK_BIT*/ VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	rasterizer.lineWidth = 1.0f;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {0};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = 0;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Dynamic state in order to change viewport size without having to recreate the whole pipeline
	VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamic_state = {0};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = dynamic_states;

	// Vertex input
	VkVertexInputBindingDescription binding_description = {0};
	binding_description.binding = 0;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	binding_description.stride = sizeof(vertex_3d);

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	// TODO not hardcode this dude
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.vertexAttributeDescriptionCount = attribute_count;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.pVertexAttributeDescriptions = attributes;

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	// Color blending
	VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {0};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f; // Optional
	color_blending.blendConstants[1] = 0.0f; // Optional
	color_blending.blendConstants[2] = 0.0f; // Optional
	color_blending.blendConstants[3] = 0.0f; // Optional

	// Depth and stencil testing
	VkPipelineDepthStencilStateCreateInfo depth_stencil = {0};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.minDepthBounds = 0.0f; // Optional
	depth_stencil.maxDepthBounds = 1.0f; // Optional
	depth_stencil.stencilTestEnable = VK_FALSE;
	// depth_stencil.front = 0; // Optional
	// depth_stencil.back = 0;  // Optional

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &out_pipeline->descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 0; // Optional
	pipeline_layout_info.pPushConstantRanges = 0;	 // Optional
	VK_CHECK(vkCreatePipelineLayout(device->handle,
									&pipeline_layout_info,
									allocator,
									&out_pipeline->layout));

	// Finally creating the pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = stage_count;
	pipeline_info.pStages = shader_stages;

	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state_info;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;

	// Fixed function state
	pipeline_info.layout = out_pipeline->layout;
	pipeline_info.renderPass = renderpass->handle;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	VK_CHECK(vkCreateGraphicsPipelines(device->handle,
									   VK_NULL_HANDLE,
									   1,
									   &pipeline_info,
									   allocator,
									   &out_pipeline->handle));
	EN_DEBUG("Graphics pipeline created.");
	return true;
}

void vulkan_pipeline_bind(vulkan_command_buffer *command_buffer, vulkan_pipeline *pipeline)
{
	vkCmdBindPipeline(command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
}

b8 vulkan_pipeline_create_descriptor_pool(vulkan_pipeline *pipeline)
{
	VkDescriptorPoolSize pool_sizes[2];

	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = (uint32_t)(pipeline->frames_in_flight);
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = (uint32_t)(pipeline->frames_in_flight);

	VkDescriptorPoolCreateInfo pool_info;
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 2;
	pool_info.pPoolSizes = pool_sizes;
	pool_info.maxSets = (uint32_t)(pipeline->frames_in_flight);

	VK_CHECK(vkCreateDescriptorPool(pipeline->device->handle, &pool_info, pipeline->allocator, &pipeline->descriptor_pool));
	return true;
}

b8 vulkan_pipeline_create_descriptor_sets(
	const VkImageView *view,
	const VkSampler *sampler,
	vulkan_pipeline *pipeline)
{
	// This surely breaks something I guess
	VkDescriptorSetLayout *layouts = darray_create(VkDescriptorSetLayout);
	for (unsigned int i = 0; i < pipeline->frames_in_flight; i++)
	{
		darray_push_back(layouts, pipeline->descriptor_set_layout);
	}

	VkDescriptorSetAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = pipeline->descriptor_pool;
	alloc_info.descriptorSetCount = (uint32_t)(pipeline->frames_in_flight);
	alloc_info.pSetLayouts = layouts;

	darray_resize(&pipeline->descriptor_sets, pipeline->frames_in_flight);
	VK_CHECK(vkAllocateDescriptorSets(pipeline->device->handle, &alloc_info, pipeline->descriptor_sets));

	for (unsigned int i = 0; i < pipeline->frames_in_flight; i++)
	{
		// TODO KEINE AHNUNG WAS WIR MIT DEM UNIFORM BUFFER HIER MACHEN
		//  VkDescriptorBufferInfo buffer_info;
		//  buffer_info.buffer = uniformBuffer.m_Buffers[i]->m_Handle;
		//  buffer_info.offset = 0;
		//  buffer_info.range = sizeof(UniformBufferObject);

		// VkDescriptorImageInfo imageInfo{};
		// imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// imageInfo.imageView = imageView;
		// imageInfo.sampler = sampler;

		// std::vector<VkWriteDescriptorSet> descriptorWrites;
		// descriptorWrites.resize(2);

		// descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// descriptorWrites[0].dstSet = m_DescriptorSets[i];
		// descriptorWrites[0].dstBinding = 0;
		// descriptorWrites[0].dstArrayElement = 0;
		// descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		// descriptorWrites[0].descriptorCount = 1;
		// descriptorWrites[0].pBufferInfo = &buffer_info;

		// descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// descriptorWrites[1].dstSet = m_DescriptorSets[i];
		// descriptorWrites[1].dstBinding = 1;
		// descriptorWrites[1].dstArrayElement = 0;
		// descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		// descriptorWrites[1].descriptorCount = 1;
		// descriptorWrites[1].pImageInfo = &imageInfo;

		// vkUpdateDescriptorSets(m_Device.m_LogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
	darray_destroy(layouts);
	return true;
}

b8 vulkan_pipeline_create_descriptor_set_layout(vulkan_pipeline *pipeline)
{
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding.pImmutableSamplers = 0; // Optional

	VkDescriptorSetLayoutBinding sampler_layout_binding;
	sampler_layout_binding.binding = 1;
	sampler_layout_binding.descriptorCount = 1;
	sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_binding.pImmutableSamplers = 0;
	sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding layout_bindings[2];
	layout_bindings[0] = ubo_layout_binding;
	layout_bindings[1] = sampler_layout_binding;

	VkDescriptorSetLayoutCreateInfo layout_info;
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 2;
	layout_info.pBindings = layout_bindings;

	VK_CHECK(vkCreateDescriptorSetLayout(pipeline->device->handle,
										 &layout_info,
										 pipeline->allocator,
										 &pipeline->descriptor_set_layout));
	return true;
}

void vulkan_pipeline_destroy(vulkan_pipeline *pipeline)
{
	vkDestroyDescriptorPool(pipeline->device->handle, pipeline->descriptor_pool, 0);
	vkDestroyDescriptorSetLayout(pipeline->device->handle, pipeline->descriptor_set_layout, pipeline->allocator);

	vkDestroyPipeline(pipeline->device->handle, pipeline->handle, pipeline->allocator);
	vkDestroyPipelineLayout(pipeline->device->handle, pipeline->layout, pipeline->allocator);
	ezero_out(pipeline, sizeof(vulkan_pipeline));
	EN_DEBUG("Vulkan pipeline destroyed.");
}