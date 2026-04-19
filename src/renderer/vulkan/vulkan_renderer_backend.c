#include "vulkan_renderer_backend.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_framebuffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_buffer.h"
#include "vulkan_utils.h"
#include "vulkan_shader_module.h"

#include "renderer/renderbuffers.h"

#include "math/math_types.h"

#include "core/platform.h"
#include "core/application.h"
#include "containers/darray.h"
#include "memory/ememory.h"

static vulkan_context *context;

b8 recreate_swapchain(renderer_backend *backend);
b8 regenerate_command_buffers(vulkan_context* context);

static u16 cached_framebuffer_width = 0;
static u16 cached_framebuffer_height = 0;

b8 vulkan_renderer_backend_initialize(
	renderer_backend *backend,
	const char *application_name,
	struct platform_state *plat_state)
{
	EN_DEBUG("Intializing Vulkan Renderer...");
	context = eallocate(sizeof(vulkan_context), MEMORY_TYPE_VULKAN_RENDERER);
	ezero_out(context, sizeof(vulkan_context));

	context->allocator = 0;

	// Create vulkan instance
	vulkan_instance_config config = {0};
	config.extensions = darray_create(i8 *);

	const char *plat_ext = platform_get_vulkan_extensions();
	darray_push_back(config.extensions, plat_ext);
	darray_push_back(config.extensions, &VK_KHR_SURFACE_EXTENSION_NAME);
	darray_push_back(config.extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	config.validation_layers = darray_create(const i8 *);
	darray_push_back(config.validation_layers, &"VK_LAYER_KHRONOS_validation");

	if (!vulkan_instance_create(&config, context->allocator, &context->instance))
	{
		EN_FATAL("vulkan_renderer_backend_initialize - Failed to create vulkan instance.");
		return false;
	}
	// Free instance config darrays
	darray_destroy(config.extensions);
	darray_destroy(config.validation_layers);

	// Create surface
	if (!vulkan_surface_create(
			&plat_state->window_handle,
			&plat_state->window_hinstance,
			&context->instance,
			context->allocator,
			&context->surface))
	{
		EN_ERROR("Failed to create surface.");
		return false;
	}

	// Create device
	if (!vulkan_device_create(&context->surface, &context->instance, VK_TRUE, VK_TRUE, &context->device))
	{
		EN_ERROR("vulkan_renderer_backend_initialize - Failed to create vulkan device.");
		return false;
	}

	// Create swapchain
	if (!vulkan_swapchain_create(plat_state->width, plat_state->height,
								 &context->device,
								 context->allocator,
								 &context->swapchain))
	{
		EN_ERROR("vulkan_renderer_backend_initialize - Failed to create vulkan swapchain.");
		return false;
	}

	// // Create vulkan image THIS IS TEXTURE STUFF WATCH KOHI SPAST
	// if (!vulkan_image_create(plat_state->width,
	// 						 plat_state->height,
	// 						 VK_FORMAT_R8G8B8A8_SRGB,
	// 						 VK_IMAGE_TILING_OPTIMAL,
	// 						 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	// 						 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	// 						 &context->device,
	// 						 context->allocator,
	// 						 &context->image))
	// {
	// 	EN_ERROR("Failed to create vulkan image.");
	// 	return false;
	// }

	// Vertex buffer
	vertex_3d *vertices = darray_create_size(vertex_3d, 4);
	vertex_3d verts[4] = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
	};

	for (u32 i = 0; i < 4; ++i)
	{
		darray_push_back(vertices, verts[i]);
	}

	// if (!vertex_buffer_create(vertices, &context->device, context->allocator, &context->vertex_buffer))
	// {
	// 	EN_ERROR("vulkan_renderer_backend_initialize - Failed to create vertex buffer.");
	// 	return false;
	// }

	darray_destroy(vertices);
	u32 *indices = darray_create_size(u32, 6);
	u32 ind[6] = {0, 1, 2, 2, 3, 1};
	for (u32 i = 0; i < 6; i++)
	{
		darray_push_back(indices, ind[i]);
	}

	// if (!index_buffer_create(indices, &context->device, context->allocator, &context->index_buffer))
	// {
	// 	EN_ERROR("vulkan_renderer_backend_initialize - Failed to create index buffer.");
	// 	return false;
	// }
	darray_destroy(indices);

	if (!vulkan_renderpass_create(&context->device, context->swapchain.surface_format.format, context->allocator, &context->renderpass))
	{
		EN_ERROR(" - vulkan_renderer_backend_initialize - Failed to create vulkan renderpass in backend.");
		return false;
	}

	context->framebuffer_width = context->swapchain.width;
	context->framebuffer_height = context->swapchain.height;
	cached_framebuffer_width = 0;
	cached_framebuffer_height = 0;
	// Create framebuffer
	if (!vulkan_swapchain_create_framebuffers(&context->swapchain, &context->renderpass))
	{
		EN_ERROR("vulkan_renderer_backend_initialize - Failed to create framebuffers.");
		return false;
	}

	// if (!vulkan_object_shader_create(context, &context->object_shader))
	// {
	// 	EN_ERROR("Failed to create vulkan object shader. Aborting renderer initialization.");
	// 	return false;
	// }

	// Create descriptor pool and sets
	// vulkan_pipeline_create_descriptor_pool(&context->object_shader.pipeline);
	// vulkan_pipeline_create_descriptor_sets(&context->image.view, &context->image.sampler, &context->object_shader.pipeline);

	// Create per swapchain image resources
	context->command_buffers = darray_create_size(vulkan_command_buffer, context->swapchain.image_count);
	context->render_finished_semaphores = darray_create_size(vulkan_semaphore, context->swapchain.image_count);

	for (u32 i = 0; i < context->swapchain.image_count; i++)
	{
		if (!vulkan_command_buffer_create(
				&context->device,
				context->device.command_pool,
				&context->command_buffers[i]))
		{
			EN_ERROR("Failed to create vulkan command buffer in vulkan backend.");
			return false;
		}

		vulkan_semaphore_create(&context->device, context->allocator, &context->render_finished_semaphores[i]);
	}

	// Create per frame in flight resources
	context->image_available_semaphores = darray_create_size(vulkan_semaphore, context->swapchain.max_frames_in_flight);
	context->in_flight_fences = darray_create_size(vulkan_fence, context->swapchain.max_frames_in_flight);
	for (u32 i = 0; i < context->swapchain.max_frames_in_flight; i++)
	{
		vulkan_semaphore_create(&context->device, context->allocator, &context->image_available_semaphores[i]);
		vulkan_fence_create(&context->device, context->allocator, &context->in_flight_fences[i]);
	}
	return true;
}

b8 vulkan_renderer_backend_begin_frame(struct renderer_backend *backend, f32 delta_time)
{
	if (context->recreating_swapchain)
	{
		VkResult result = vkDeviceWaitIdle(context->device.handle);
		if (result != VK_SUCCESS)
		{
			EN_ERROR("vulkan_renderer_backend_begin_frame - wait idle(1) failed.");
			return false;
		}
		EN_INFO("Currently recreating swapchain, booting.");
		return false;
	}

	if (context->framebuffer_generation != context->framebuffer_last_generation)
	{
		VkResult result = vkDeviceWaitIdle(context->device.handle);
		if (result != VK_SUCCESS)
		{
			EN_ERROR("vulkan_renderer_backend_begin_frame - wait idle(2) failed.");
			return false;
		}
		if (!recreate_swapchain(backend))
		{
			return false;
		}
		return false;
	}

	// Wait for the previous frame to finish
	u32 frame_in_flight_index = context->current_frame_in_flight_index;
	vkWaitForFences(
		context->device.handle,
		1,
		&context->in_flight_fences[frame_in_flight_index].handle,
		VK_TRUE,
		UINT64_MAX);

	if (!vulkan_swapchain_acquire_next_image(
			&context->swapchain,
			&context->renderpass,
			context->image_available_semaphores[frame_in_flight_index].handle))
	{
		EN_DEBUG("Swapchain recreation. Booting.");
		return false;
	}

	// Reset the fence as it was signaled from the previous frame
	// Only reset after swapchain image was acquired, because swapchain_acquire
	// may trigger a recreate but fence is never reset in that case
	vkResetFences(
		context->device.handle,
		1,
		&context->in_flight_fences[frame_in_flight_index].handle);
	u32 swapchain_image_index = context->swapchain.current_swapchain_image_index;

	// Reset command buffer
	vulkan_command_buffer *cb = &context->command_buffers[swapchain_image_index];
	vkResetCommandBuffer(cb->handle, 0);

	if (!vulkan_command_buffer_begin(cb))
	{
		EN_ERROR("Failed to record command buffer.");
		return false;
	}

	// vulkan_pipeline_bind(cb, &context->object_shader.pipeline);

	vulkan_renderpass_begin(
		cb,
		context->swapchain.extent,
		&context->swapchain.framebuffers[swapchain_image_index],
		&context->renderpass);

	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)(context->swapchain.width);
	viewport.height = (float)(context->swapchain.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cb->handle, 0, 1, &viewport);

	// Create scissor
	VkRect2D scissor = {0};
	scissor.offset = (VkOffset2D){0, 0};
	scissor.extent = context->swapchain.extent;
	vkCmdSetScissor(cb->handle, 0, 1, &scissor);

	// Bind Buffers
	VkBuffer vertexBuffers[] = {context->vertex_buffer.internal_buffer.handle};
	VkDeviceSize offsets[] = {0};
	// vkCmdBindVertexBuffers(cb->handle,
	// 					   0,
	// 					   1,
	// 					   vertexBuffers,
	// 					   offsets);
	// vkCmdBindIndexBuffer(cb->handle,
	// 					 context->index_buffer.internal_buffer.handle,
	// 					 0,
	// 					 VK_INDEX_TYPE_UINT32);

	// vkCmdBindDescriptorSets(context->command_buffers[context->swapchain.current_frame].handle,
	// 						VK_PIPELINE_BIND_POINT_GRAPHICS,
	// 						context->object_shader.pipeline.layout,
	// 						0,
	// 						1,
	// 						&context->object_shader.pipeline.descriptor_sets[context->swapchain.current_frame],
	// 						0,
	// 						0);
	// vkCmdDrawIndexed(cb->handle,
	// 				 (uint32_t)(context->index_buffer.internal_buffer.size),
	// 				 1,
	// 				 0,
	// 				 0,
	// 				 0);
	return true;
}

b8 vulkan_renderer_backend_end_frame(struct renderer_backend *backend, f32 delta_time)
{
	u32 frame_in_flight_index = context->current_frame_in_flight_index;
	u32 swapchain_image_index = context->swapchain.current_swapchain_image_index;
	vulkan_command_buffer *cb = &context->command_buffers[swapchain_image_index];

	if (!vulkan_renderpass_end(cb))
	{
		EN_ERROR("Failed to end renderpass.");
		return false;
	}

	if (!vulkan_command_buffer_end(cb))
	{
		EN_ERROR("Failed to end command buffer.");
		return false;
	}

	// Submitting the command buffer
	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = {context->image_available_semaphores[frame_in_flight_index].handle};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = waitSemaphores;
	submit_info.pWaitDstStageMask = waitStages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cb->handle;

	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &context->render_finished_semaphores[swapchain_image_index].handle;

	VkResult result = vkQueueSubmit(context->device.graphics_queue,
									1,
									&submit_info,
									context->in_flight_fences[frame_in_flight_index].handle);
	if (result != VK_SUCCESS)
	{
		EN_ERROR("vkQueueSubmit failed with result: %s.", vulkan_result_string(result, true));
		return false;
	}

	vulkan_swapchain_present(
		&context->swapchain,
		&context->render_finished_semaphores[swapchain_image_index].handle);

	context->current_frame_in_flight_index = (context->current_frame_in_flight_index + 1) % context->swapchain.max_frames_in_flight;
	return true;
}

void vulkan_renderer_backend_shutdown(struct renderer_backend *backend)
{
	// Destroy vulkan objects in the reverse order they were created
	vkDeviceWaitIdle(context->device.handle);

	// Destroy command buffers
	for (unsigned int i = 0; i < context->swapchain.max_frames_in_flight; i++)
	{
		vulkan_command_buffer_destroy(&context->command_buffers[i]);
	}

	// Also destroy command_buffer's darray
	darray_destroy(context->command_buffers);

	// Destroy debug utils messenger
#ifdef _DEBUG
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance.handle, "vkDestroyDebugUtilsMessengerEXT");
	if (func != 0)
	{
		func(context->instance.handle, context->instance.debug_messenger, context->allocator);
	}
#endif
	// vulkan_object_shader_destroy(&context->object_shader);
	// vulkan_pipeline_destroy(&context->object_shader.pipeline);
	//vertex_buffer_destroy(&context->vertex_buffer);
	//vulkan_image_destroy(&context->image);
	vulkan_swapchain_destroy_framebuffers(&context->swapchain, &context->renderpass);
	vulkan_swapchain_destroy(&context->swapchain);
	vulkan_renderpass_destroy(&context->renderpass);

	// Destroy sync objects
	for (u32 i = 0; i < context->swapchain.image_count; i++)
	{
		vulkan_semaphore_destroy(&context->render_finished_semaphores[i]);
	}

	for (u32 i = 0; i < context->swapchain.max_frames_in_flight; i++)
	{
		vulkan_semaphore_destroy(&context->image_available_semaphores[i]);
		vulkan_fence_destroy(&context->in_flight_fences[i]);
	}

	darray_destroy(context->image_available_semaphores);
	darray_destroy(context->render_finished_semaphores);
	darray_destroy(context->in_flight_fences);

	vulkan_device_destroy(&context->device);
	vulkan_surface_destroy(&context->surface);
	vulkan_instance_destroy(&context->instance);

	efree(context, sizeof(vulkan_context), MEMORY_TYPE_VULKAN_RENDERER);

	EN_DEBUG("Destroying Vulkan Renderer...");
}

b8 vulkan_renderer_backend_on_resize(struct renderer_backend *backend, u16 width, u16 height)
{
	vulkan_context *c = context;
	cached_framebuffer_width = width;
	cached_framebuffer_height = height;
	c->framebuffer_generation++;
	return true;
}

b8 recreate_swapchain(renderer_backend *backend)
{
	if(context->recreating_swapchain) {
		EN_DEBUG("recreate_swapchain - already recreating. Booting.");
		return false;
	}

  	// if (context->framebuffer_width == 0 || context->framebuffer_height == 0) {
    //     EN_DEBUG("recreate_swapchain called when window is < 1 in a dimension. Booting.");
    //     return FALSE;
    // }

	context->recreating_swapchain = true;

	vkDeviceWaitIdle(context->device.handle);

    vulkan_swapchain_destroy_framebuffers(&context->swapchain, &context->renderpass);

	vulkan_device_query_swapchain_support(&context->device, context->device.physical_device);

    // 2. Dann Swapchain recreaten (zerstört ImageViews, erstellt neue)
    vulkan_swapchain_recreate(
        &context->swapchain,
        cached_framebuffer_width,
        cached_framebuffer_height);

    vulkan_swapchain_create_framebuffers(&context->swapchain, &context->renderpass);
	context->framebuffer_last_generation = context->framebuffer_generation;

	regenerate_command_buffers(context);

	context->framebuffer_width = cached_framebuffer_width;
	context->framebuffer_height = cached_framebuffer_height;
	cached_framebuffer_width = 0;
	cached_framebuffer_height = 0;

	context->recreating_swapchain = false;
	return true;
}

b8 regenerate_command_buffers(vulkan_context* context) {
		// Destroy command buffers
	for (unsigned int i = 0; i < context->swapchain.image_count; i++)
	{
		vulkan_command_buffer_destroy(&context->command_buffers[i]);
	}

	for (u32 i = 0; i < context->swapchain.image_count; i++)
	{
		if (!vulkan_command_buffer_create(
				&context->device,
				context->device.command_pool,
				&context->command_buffers[i]))
		{
			return false;
		}
	}
	return true;
}