// #include <stdio.h>

#include "vulkan_object_shader.h"
#include "vulkan_pipeline.h"
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

    // dynamically create VertexInputAttributeDescription
    const u32 attribute_count = 1;
    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];

    VkFormat formats[attribute_count];
    formats[0] = VK_FORMAT_R32G32B32_SFLOAT; // vec3 position
    //formats[1] = VK_FORMAT_R32G32B32_SFLOAT; // vec3 color

    u64 sizes[attribute_count];
    sizes[0] = sizeof(vec3);
    //sizes[1] = sizeof(vec3);

    u32 offset = 0;
    for (u32 i = 0; i < attribute_count; i++)
    {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        stage_create_infos[i].sType = out_object_shader->stages[i].shader_stage_create_info.sType;
        stage_create_infos[i] = out_object_shader->stages[i].shader_stage_create_info;
    }

    VkViewport viewport = {0};
    viewport.height = context->framebuffer_height;
    viewport.width = context->framebuffer_width;
    viewport.x = 0;
    viewport.y = 0;
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
            0,
            0,
            viewport,
            scissor,
            false,
            &context->object_shader.pipeline))
    {
        EN_ERROR("Failed to create vulkan graphics pipeline.");
        return false;
    }

    return true;
}

void vulkan_object_shader_use(vulkan_object_shader *shader, vulkan_command_buffer* cb)
{
    vulkan_pipeline_bind(cb, &shader->pipeline);
}

void vulkan_object_shader_destroy(vulkan_object_shader *shader_module)
{
    vulkan_pipeline_destroy(&shader_module->pipeline);
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        vkDestroyShaderModule(
            shader_module->device->handle,
            shader_module->stages[i].handle,
            shader_module->allocator);
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