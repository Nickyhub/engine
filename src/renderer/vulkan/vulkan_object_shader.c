// #include <stdio.h>

#include "vulkan_object_shader.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_utils.h"

#include "memory/ememory.h"
#include "core/file.h"

#define MATERIAL_SHADER_NAME "MaterialShader"

b8 create_shader_module(const char *name,
                        char *shader_type_str,
                        VkShaderStageFlags stage_type,
                        const vulkan_device *device,
                        const VkAllocationCallbacks *allocator,
                        vulkan_shader_stage *out_shader_stage);

b8 vulkan_object_shader_create(
    vulkan_context *context,
    vulkan_object_shader *out_object_shader)
{
    out_object_shader->device = &context->device;
    out_object_shader->allocator = context->allocator;
    char *shader_type_str[2] = {".vert\0", ".frag\0"};
    VkShaderStageFlagBits stage_types[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    ezero_out(out_object_shader->stages, sizeof(vulkan_shader_stage) * OBJECT_SHADER_STAGE_COUNT);

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        if (!create_shader_module(
                MATERIAL_SHADER_NAME,
                shader_type_str[i],
                stage_types[i],
                &context->device,
                context->allocator,
                &out_object_shader->stages[i]))
        {
            EN_ERROR("Failed to create %s shader module for %s", shader_type_str, MATERIAL_SHADER_NAME);
            return false;
        }
    }

    // TODO Move descriptors here
    VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = 0; // Optional

    VkDescriptorSetLayoutBinding layout_bindings[1];
    layout_bindings[0] = ubo_layout_binding;

    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = layout_bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(out_object_shader->device->handle,
                                         &layout_info,
                                         context->allocator,
                                         &out_object_shader->global_descriptor_set_layout));

    VkDescriptorPoolSize pool_sizes[1];

    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = (uint32_t)(context->swapchain.image_count);

    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = (uint32_t)(context->swapchain.image_count);

    VK_CHECK(vkCreateDescriptorPool(
        context->device.handle,
        &pool_info, context->allocator,
        &out_object_shader->descriptor_pool));

    // dynamically create VertexInputAttributeDescription
    const u32 attribute_count = 1;
    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];

    VkFormat formats[attribute_count];
    formats[0] = VK_FORMAT_R32G32B32_SFLOAT; // vec3 position
    // formats[1] = VK_FORMAT_R32G32B32_SFLOAT; // vec3 color

    u64 sizes[attribute_count];
    sizes[0] = sizeof(vec3);
    // sizes[1] = sizeof(vec3);

    u32 offset = 0;
    for (u32 i = 0; i < attribute_count; i++)
    {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    // Descriptor set layouts.
    const i32 descriptor_set_layout_count = 1;
    VkDescriptorSetLayout layouts[1] = {
        out_object_shader->global_descriptor_set_layout};

    VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        stage_create_infos[i].sType = out_object_shader->stages[i].shader_stage_create_info.sType;
        stage_create_infos[i] = out_object_shader->stages[i].shader_stage_create_info;
    }

    VkViewport viewport = {0};
    viewport.y = (float)context->framebuffer_height; // y startet unten
    viewport.x = 0;
    viewport.width = (float)context->framebuffer_width;
    viewport.height = -(float)context->framebuffer_height; // negativ = flip
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor = {0};
    scissor.extent = context->swapchain.extent;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    // Create the graphics pipeline from here
    if (!vulkan_pipeline_create(
            context,
            &context->renderpass,
            attribute_count,
            attribute_descriptions,
            OBJECT_SHADER_STAGE_COUNT,
            stage_create_infos,
            descriptor_set_layout_count,
            layouts,
            viewport,
            scissor,
            false,
            &context->object_shader.pipeline))
    {
        EN_ERROR("Failed to create vulkan graphics pipeline.");
        return false;
    }

    if (!vulkan_buffer_create(
            sizeof(global_uniform_object),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true,
            &context->device,
            context->allocator,
            &out_object_shader->global_uniform_buffer))
    {
        EN_ERROR("Vulkan buffer creation failed for object shader.");
        return false;
    }

    // Allocate global desctriptor sets
    VkDescriptorSetLayout global_layouts[3] = {
        out_object_shader->global_descriptor_set_layout,
        out_object_shader->global_descriptor_set_layout,
        out_object_shader->global_descriptor_set_layout};

    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.descriptorPool = out_object_shader->descriptor_pool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pSetLayouts = global_layouts;
    VK_CHECK(vkAllocateDescriptorSets(
        context->device.handle,
        &alloc_info, out_object_shader->global_descriptor_sets));

    return true;
}

void vulkan_object_shader_use(vulkan_object_shader *shader, vulkan_command_buffer *cb)
{
    vulkan_pipeline_bind(cb, &shader->pipeline);
}

void vulkan_object_shader_destroy(vulkan_object_shader *object_shader)
{
    vulkan_buffer_destroy(&object_shader->global_uniform_buffer);

    vulkan_pipeline_destroy(&object_shader->pipeline);

    vkDestroyDescriptorPool(
        object_shader->device->handle,
        object_shader->descriptor_pool,
        object_shader->allocator);

    vkDestroyDescriptorSetLayout(
        object_shader->device->handle,
        object_shader->global_descriptor_set_layout,
        object_shader->allocator);

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        vkDestroyShaderModule(
            object_shader->device->handle,
            object_shader->stages[i].handle,
            object_shader->allocator);
    }
}

b8 create_shader_module(const char *name,
                        char *shader_type_str,
                        VkShaderStageFlags stage_type,
                        const vulkan_device *device,
                        const VkAllocationCallbacks *allocator,
                        vulkan_shader_stage *out_shader_stage)
{
    // Build the file path based on the name
    char file_path[256];
    ezero_out(file_path, sizeof(char) * 256);

    sprintf_s(file_path, 256, "assets/shaders/%s%s.spv", name, shader_type_str);
    efile shader_file;
    file_open(&shader_file, file_path, FILE_MODE_READ, true);

    i8 *shader_source = darray_create_size(i8, shader_file.size);
    file_read_all_bytes(&shader_file, shader_source);
    file_close(&shader_file);

    ezero_out(&out_shader_stage->create_info, sizeof(VkShaderModule));
    out_shader_stage->create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    out_shader_stage->create_info.codeSize = shader_file.size;
    out_shader_stage->create_info.pCode = (const u32 *)shader_source;

    VkPipelineShaderStageCreateInfo shader_stage_create_info = {0};
    out_shader_stage->shader_stage_create_info;

    VK_CHECK(vkCreateShaderModule(
        device->handle,
        &out_shader_stage->create_info,
        allocator,
        &out_shader_stage->handle));

    // zero out Memory just to be sure to avoid seg faults
    ezero_out(&out_shader_stage->shader_stage_create_info, sizeof(VkPipelineShaderStageCreateInfo));
    out_shader_stage->shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    out_shader_stage->shader_stage_create_info.stage = stage_type;
    out_shader_stage->shader_stage_create_info.module = out_shader_stage->handle;
    out_shader_stage->shader_stage_create_info.pName = "main";

    EN_INFO("Created shader module %s successfully.", file_path);

    darray_destroy(shader_source);
    return true;
}

void vulkan_object_shader_update_global_state(vulkan_context *context, vulkan_object_shader *object_shader)
{
    u32 image_index = context->swapchain.current_swapchain_image_index;
    VkCommandBuffer cb = context->command_buffers[image_index].handle;
    VkDescriptorSet global_descriptor = object_shader->global_descriptor_sets[image_index];

    // Bind the global descriptor set to be updated.
    vkCmdBindDescriptorSets(
        cb,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        object_shader->pipeline.layout,
        0,
        1,
        &global_descriptor,
        0,
        0);

    u32 range = sizeof(global_uniform_object);
    u64 offset = 0;

    vulkan_buffer_load_data(
        &object_shader->global_uniform_buffer,
        offset,
        range,
        0,
        &object_shader->global_ubo,
        &context->device,
        context->allocator);

    VkDescriptorBufferInfo buffer_info = {0};
    buffer_info.buffer = object_shader->global_uniform_buffer.handle;
    buffer_info.offset = offset;
    buffer_info.range = range;

    VkWriteDescriptorSet descriptor_write = {0};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = object_shader->global_descriptor_sets[image_index];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(context->device.handle, 1, &descriptor_write, 0, 0);
}