// #include <stdio.h>

#include "vulkan_material_shader.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_utils.h"

#include "memory/ememory.h"
#include "core/file.h"
#include "math/emath.h"

#include "systems/texture_system.h"

#define MATERIAL_SHADER_NAME "MaterialShader"
b8 create_global_descriptor_set(vulkan_context *context, vulkan_material_shader *out_shader);
b8 create_object_descriptor_set(vulkan_context *context, vulkan_material_shader *out_shader);

b8 create_shader_module(const char *name,
                        char *shader_type_str,
                        VkShaderStageFlags stage_type,
                        const vulkan_device *device,
                        const VkAllocationCallbacks *allocator,
                        vulkan_shader_stage *out_shader_stage);

b8 vulkan_material_shader_create(
    vulkan_context *context,
    vulkan_material_shader *out_material_shader)
{
    out_material_shader->device = &context->device;
    out_material_shader->allocator = context->allocator;

    char *shader_type_str[2] = {".vert\0", ".frag\0"};
    VkShaderStageFlagBits stage_types[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    ezero_out(out_material_shader->stages, sizeof(vulkan_shader_stage) * VULKAN_MATERIAL_SHADER_STAGE_COUNT);

    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_STAGE_COUNT; i++)
    {
        if (!create_shader_module(
                MATERIAL_SHADER_NAME,
                shader_type_str[i],
                stage_types[i],
                &context->device,
                context->allocator,
                &out_material_shader->stages[i]))
        {
            EN_ERROR("Failed to create %s shader module for %s", shader_type_str[i], MATERIAL_SHADER_NAME);
            return false;
        }
    }

    if (!create_global_descriptor_set(context, out_material_shader))
    {
        EN_ERROR("Failed to create global descriptor set for material shader.");
        return false;
    }

    if (!create_object_descriptor_set(context, out_material_shader))
    {
        EN_ERROR("Failed to create object descriptor set for material shader.");
        return false;
    }

    // Vertex attributes
#define ATTRIBUTE_COUNT 2
    VkVertexInputAttributeDescription attribute_descriptions[ATTRIBUTE_COUNT];
    VkFormat formats[ATTRIBUTE_COUNT] = {VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT};
    u64 sizes[ATTRIBUTE_COUNT] = {sizeof(vec3), sizeof(vec2)};
    u32 offset = 0;
    for (u32 i = 0; i < ATTRIBUTE_COUNT; i++)
    {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    VkPipelineShaderStageCreateInfo stage_create_infos[VULKAN_MATERIAL_SHADER_STAGE_COUNT];
    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_STAGE_COUNT; i++)
        stage_create_infos[i] = out_material_shader->stages[i].shader_stage_create_info;

    VkDescriptorSetLayout layouts[2] = {
        out_material_shader->global_descriptor_set_layout,
        out_material_shader->object_descriptor_set_layout};

    VkViewport viewport = {0};
    viewport.x = 0;
    viewport.y = (float)context->framebuffer_height;
    viewport.width = (float)context->framebuffer_width;
    viewport.height = -(float)context->framebuffer_height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor = {0};
    scissor.extent = context->swapchain.extent;

    if (!vulkan_pipeline_create(
            context,
            &context->renderpass,
            ATTRIBUTE_COUNT,
            attribute_descriptions,
            VULKAN_MATERIAL_SHADER_STAGE_COUNT,
            stage_create_infos,
            2,
            layouts,
            viewport,
            scissor,
            false,
            &context->material_shader.pipeline))
    {
        EN_ERROR("Failed to create vulkan graphics pipeline.");
        return false;
    }

    if (!vulkan_buffer_create(
            sizeof(global_uniform_object) * 3,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true,
            &context->device,
            context->allocator,
            &out_material_shader->global_uniform_buffer))
    {
        EN_ERROR("Vulkan buffer creation failed for material shader.");
        return false;
    }

    if (!vulkan_buffer_create(
            sizeof(material_uniform_object) * VULKAN_MAX_MATERIAL_COUNT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true,
            &context->device,
            context->allocator,
            &out_material_shader->object_uniform_buffer))
    {
        EN_ERROR("Material instance buffer creation failed for material shader.");
        return false;
    }

    return true;
}

void vulkan_material_shader_use(vulkan_material_shader *shader, vulkan_command_buffer *cb)
{
    vulkan_pipeline_bind(cb, &shader->pipeline);
}

void vulkan_material_shader_destroy(vulkan_material_shader *material_shader)
{
    vulkan_buffer_destroy(&material_shader->global_uniform_buffer);
    vulkan_buffer_destroy(&material_shader->object_uniform_buffer);

    vulkan_pipeline_destroy(&material_shader->pipeline);

    vkDestroyDescriptorPool(
        material_shader->device->handle,
        material_shader->object_descriptor_pool,
        material_shader->allocator);

    vkDestroyDescriptorPool(
        material_shader->device->handle,
        material_shader->descriptor_pool,
        material_shader->allocator);

    vkDestroyDescriptorSetLayout(
        material_shader->device->handle,
        material_shader->global_descriptor_set_layout,
        material_shader->allocator);

    vkDestroyDescriptorSetLayout(
        material_shader->device->handle,
        material_shader->object_descriptor_set_layout,
        material_shader->allocator);

    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_STAGE_COUNT; i++)
    {
        vkDestroyShaderModule(
            material_shader->device->handle,
            material_shader->stages[i].handle,
            material_shader->allocator);
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

b8 create_global_descriptor_set(vulkan_context *context, vulkan_material_shader *out_shader)
{
    VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = 0;

    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout_binding;

    VK_CHECK(vkCreateDescriptorSetLayout(
        out_shader->device->handle,
        &layout_info,
        out_shader->allocator,
        &out_shader->global_descriptor_set_layout));

    VkDescriptorPoolSize pool_size = {0};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = (u32)context->swapchain.image_count;

    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = (u32)context->swapchain.image_count;

    VK_CHECK(vkCreateDescriptorPool(
        context->device.handle,
        &pool_info,
        out_shader->allocator,
        &out_shader->descriptor_pool));

    VkDescriptorSetLayout global_layouts[3] = {
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout};

    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = out_shader->descriptor_pool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.pSetLayouts = global_layouts;

    VK_CHECK(vkAllocateDescriptorSets(
        context->device.handle,
        &alloc_info,
        out_shader->global_descriptor_sets));

    return true;
}

b8 create_object_descriptor_set(vulkan_context *context, vulkan_material_shader *out_shader)
{
    VkDescriptorType descriptor_types[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };

    VkDescriptorSetLayoutBinding bindings[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    ezero_out(bindings, sizeof(VkDescriptorSetLayoutBinding) * VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; i++)
    {
        bindings[i].binding = i;
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = descriptor_types[i];
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layout_info.bindingCount = VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
    layout_info.pBindings = bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(
        out_shader->device->handle,
        &layout_info,
        out_shader->allocator,
        &out_shader->object_descriptor_set_layout));

    // Sampler uses
    out_shader->sampler_uses[0] = TEXTURE_USE_DIFFUSE;

    VkDescriptorPoolSize object_pool_sizes[2];
    object_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    object_pool_sizes[0].descriptorCount = VULKAN_MAX_MATERIAL_COUNT;
    object_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    object_pool_sizes[1].descriptorCount = VULKAN_MATERIAL_SHADER_SAMPLER_COUNT * VULKAN_MAX_MATERIAL_COUNT;

    VkDescriptorPoolCreateInfo object_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    object_pool_info.poolSizeCount = 2;
    object_pool_info.pPoolSizes = object_pool_sizes;
    object_pool_info.maxSets = VULKAN_MAX_MATERIAL_COUNT;
    object_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(
        context->device.handle,
        &object_pool_info,
        out_shader->allocator,
        &out_shader->object_descriptor_pool));

    return true;
}

void vulkan_material_shader_update_global_state(vulkan_context *context, vulkan_material_shader *material_shader, f32 delta_time)
{
    u32 image_index = context->swapchain.current_swapchain_image_index;
    VkCommandBuffer cb = context->command_buffers[image_index].handle;
    VkDescriptorSet global_descriptor = material_shader->global_descriptor_sets[image_index];

    u32 range = sizeof(global_uniform_object);
    u64 offset = sizeof(global_uniform_object) * context->swapchain.current_swapchain_image_index;

    vulkan_buffer_load_data(
        &material_shader->global_uniform_buffer,
        offset,
        range,
        0,
        &material_shader->global_ubo,
        &context->device,
        context->allocator);

    VkDescriptorBufferInfo buffer_info = {0};
    buffer_info.buffer = material_shader->global_uniform_buffer.handle;
    buffer_info.offset = offset;
    buffer_info.range = range;

    VkWriteDescriptorSet descriptor_write = {0};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = material_shader->global_descriptor_sets[image_index];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(context->device.handle, 1, &descriptor_write, 0, 0);

    // Bind the global descriptor set to be updated.
    vkCmdBindDescriptorSets(
        cb,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        material_shader->pipeline.layout,
        0,
        1,
        &global_descriptor,
        0,
        0);
}

void vulkan_material_shader_update_object(vulkan_context *context, vulkan_material_shader *shader, geometry_render_data data)
{
    u32 image_index = context->swapchain.current_swapchain_image_index;

    VkCommandBuffer cb = context->command_buffers[image_index].handle;
    vkCmdPushConstants(
        cb,
        context->material_shader.pipeline.layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(mat4), &data.model);

    vulkan_material_shader_instance_state *object_state = &shader->instance_states[data.material->internal_id];
    VkDescriptorSet object_descriptor_set = object_state->descriptor_sets[image_index];

    VkWriteDescriptorSet descriptor_writes[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    ezero_out(descriptor_writes, sizeof(VkWriteDescriptorSet) * VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
    u32 descriptor_count = 0;
    u32 descriptor_index = 0;

    // Descriptor 0  - Uniform buffer
    u32 range = sizeof(material_uniform_object);
    u64 offset = sizeof(material_uniform_object) * data.material->internal_id;
    material_uniform_object obo = {0};

    // TODO: get diffuse colour from material.
    // static f32 accumulator = 0.0f;
    // accumulator += context->frame_delta_time;
    // f32 s = (esin(accumulator) + 1.0f) / 2.0f;
    // obo.diffuse_color = (vec4){s * 0.2, s * 0.5, s, 1.0f};

    obo.diffuse_color = data.material->diffuse_colour;

    vulkan_buffer_load_data(
        &shader->object_uniform_buffer,
        offset,
        range,
        0,
        &obo,
        &context->device, context->allocator);

    // Only do this if the descriptor has not yet been updated.
    u32 *global_ubo_generation = &object_state->descriptor_states[descriptor_index].generations[image_index];
    if (*global_ubo_generation == INVALID_ID || *global_ubo_generation != data.material->generation)
        {
            VkDescriptorBufferInfo buffer_info = {0};
            buffer_info.buffer = shader->object_uniform_buffer.handle;
            buffer_info.offset = offset;
            buffer_info.range = range;

            VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            descriptor.dstSet = object_descriptor_set;
            descriptor.dstBinding = descriptor_index;
            descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor.descriptorCount = 1;
            descriptor.pBufferInfo = &buffer_info;

            descriptor_writes[descriptor_count] = descriptor;
            descriptor_count++;

            // Update the frame generation. In this case it is only needed once since this is a buffer.
            *global_ubo_generation = data.material->generation;
        }
    descriptor_index++;

    // TODO samplers
    const u32 sampler_count = 1;
    VkDescriptorImageInfo image_infos[1];
    for (u32 sampler_index = 0; sampler_index < sampler_count; ++sampler_index)
    {
        texture_use use = shader->sampler_uses[sampler_index];
        texture *t = 0;

        switch (use)
        {
        case TEXTURE_USE_DIFFUSE:
            t = data.material->diffuse_map.texture;
            break;
        default:
            EN_FATAL("Unable to bind sampler to unknown use.");
            return;
        }

        u32 *descriptor_generation = &object_state->descriptor_states[descriptor_index].generations[image_index];
        u32 *descriptor_id = &object_state->descriptor_states[descriptor_index].ids[image_index];

        // If the texture hasnt been loaded yet, use the default.
        if (t->generation == INVALID_ID)
        {
            t = texture_system_get_default_texture();

            // Reset the descriptor generation if using the default texture.
            *descriptor_generation = INVALID_ID;
        }

        // Check if the descriptor needs updating first
        if (t && (*descriptor_id != t->id || *descriptor_generation != t->generation || *descriptor_generation == INVALID_ID))
        {
            vulkan_texture_data *internal_data = (vulkan_texture_data *)t->internal_data;

            // Assign view and sampler
            image_infos[sampler_index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_infos[sampler_index].imageView = internal_data->image.view;
            image_infos[sampler_index].sampler = internal_data->sampler;

            VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            descriptor.dstSet = object_descriptor_set;
            descriptor.dstBinding = descriptor_index;
            descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor.descriptorCount = 1;
            descriptor.pImageInfo = &image_infos[sampler_index];

            descriptor_writes[descriptor_count] = descriptor;
            descriptor_count++;

            // Sync frame generation if notusing a default texture.
            if (t->generation != INVALID_ID)
            {
                *descriptor_generation = t->generation;
                *descriptor_id = t->id;
            }
            descriptor_index++;
        }
    }

    if (descriptor_count > 0)
    {
        vkUpdateDescriptorSets(context->device.handle, descriptor_count, descriptor_writes, 0, 0);
    }

    // Bind the descriptor set to be updated, or in case the shader changed.
    vkCmdBindDescriptorSets(
        cb,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader->pipeline.layout,
        1,
        1,
        &object_descriptor_set,
        0,
        0);
}

b8 vulkan_material_shader_acquire_resources(vulkan_context *context, struct vulkan_material_shader *shader, material *material)
{
    material->internal_id = shader->object_uniform_buffer_index;
    shader->object_uniform_buffer_index++;

    vulkan_material_shader_instance_state *object_state = &shader->instance_states[material->internal_id];

    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; i++)
    {
        for (u32 j = 0; j < 3; ++j)
        {
            object_state->descriptor_states[i].generations[j] = INVALID_ID;
            object_state->descriptor_states[i].ids[j] = INVALID_ID;
        }
    }
    // Allocate descriptor sets.
    VkDescriptorSetLayout layouts[3] = {
        shader->object_descriptor_set_layout,
        shader->object_descriptor_set_layout,
        shader->object_descriptor_set_layout};

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = shader->object_descriptor_pool;
    alloc_info.descriptorSetCount = 3; // One per frame
    alloc_info.pSetLayouts = layouts;
    VkResult result = vkAllocateDescriptorSets(context->device.handle, &alloc_info, object_state->descriptor_sets);
    if (result != VK_SUCCESS)
    {
        EN_ERROR("Error allocating descriptor sets in shader!");
        return false;
    }
    return true;
}

void vulkan_material_shader_release_resources(vulkan_context *context, struct vulkan_material_shader *shader, material *material)
{
    vulkan_material_shader_instance_state *instance_state = &shader->instance_states[material->internal_id];

    const u32 descriptor_set_count = 3;
    // Release object descriptor sets
    VkResult result = vkFreeDescriptorSets(
        context->device.handle,
        shader->object_descriptor_pool,
        descriptor_set_count,
        instance_state->descriptor_sets);

    if (result != VK_SUCCESS)
    {
        EN_ERROR("Error freeing object shader descriptor sets!");
    }

    // Invalidate IDs
    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; i++)
    {
        for (u32 j = 0; j < 3; ++j)
        {
            instance_state->descriptor_states[i].generations[j] = INVALID_ID;
            instance_state->descriptor_states[i].ids[j] = INVALID_ID;
        }
    }

    material->internal_id = INVALID_ID;
}