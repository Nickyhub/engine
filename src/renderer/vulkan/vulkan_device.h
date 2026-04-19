#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_types.inl"
#include "vulkan_instance.h"

#include "containers/darray.h"

b8 vulkan_device_create(
	vulkan_surface* surface,
	vulkan_instance* instance,
	b8 enableSamplerAnisotropy,
	b8 enable_fill_mode_non_solid,
	vulkan_device* out_device);


void vulkan_device_destroy(vulkan_device* device);

b8 vulkan_device_find_depth_format(vulkan_device* device);

b8 vulkan_device_query_swapchain_support(vulkan_device* device, VkPhysicalDevice physical_device);