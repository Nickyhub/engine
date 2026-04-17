#pragma once 

#include "vulkan_types.inl"

b8 vulkan_object_shader_create(
    const vulkan_context* context,
    vulkan_object_shader *out_object_shader);

void vulkan_shader_use(vulkan_object_shader* shader);

void vulkan_object_shader_destroy(vulkan_object_shader* shader_module);