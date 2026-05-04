#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_types.inl"
#include "vulkan_device.h"



b8 vulkan_command_buffer_create(const vulkan_device* device,
								VkCommandPool pool,
								vulkan_command_buffer* out_command_buffer);

void vulkan_command_buffer_destroy(vulkan_command_buffer* buffer);

b8 vulkan_command_buffer_begin(vulkan_command_buffer* command_buffer);

b8 vulkan_command_buffer_end(vulkan_command_buffer* command_buffer);

vulkan_command_buffer vulkan_command_buffer_begin_single_use(const vulkan_device* device,
													   VkCommandPool pool);
void vulkan_command_buffer_end_single_use(vulkan_command_buffer* command_buffer,
										  const VkQueue* queue,
										  const vulkan_device* device);
