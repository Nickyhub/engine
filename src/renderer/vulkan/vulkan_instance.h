#pragma once

#include <windows.h>
#include <vulkan/vulkan.h>
#include "vulkan_types.inl"

#include "defines.h"

typedef struct vulkan_instance_config {
	const char ** extensions; // darray
	const char ** validation_layers; // darray
	const char* name;
} vulkan_instance_config;

b8 vulkan_instance_create(
	vulkan_instance_config *config,
	const VkAllocationCallbacks* allocator,
	vulkan_instance*out_instance);

void vulkan_instance_populate_config_default(vulkan_instance_config *out_config);

void vulkan_instance_destroy(vulkan_instance* instance);

b8 vulkan_surface_create(
	HWND *window_handle,
	HINSTANCE *windows_instance,
	vulkan_instance *instance,
	VkAllocationCallbacks* allocator,
	vulkan_surface *out_surface);


void vulkan_surface_destroy(vulkan_surface *surface);