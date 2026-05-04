#pragma once

#include <windows.h>

#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"
#include "vulkan_swapchain.h"
#include "core/event.h"

#include "resources/resource_types.h"

b8 vulkan_renderer_backend_initialize(
	renderer_backend *backend,
	const char *application_name,
	struct platform_state *plat_state);

b8 vulkan_renderer_backend_begin_frame(struct renderer_backend* backend, f32 delta_time);

b8 vulkan_renderer_backend_end_frame(struct renderer_backend* backend, f32 delta_time);

b8 vulkan_renderer_backend_on_resize(struct renderer_backend* backend, u16 width, u16 height);
void vulkan_renderer_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
void vulkan_renderer_backend_shutdown(struct renderer_backend* backend);

void vulkan_renderer_backend_update_object(geometry_render_data data);

void vulkan_renderer_backend_create_texture(
    const char *name,
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8 *pixels,
    b8 has_transparency,
    texture *out_texture);

void vulkan_renderer_backend_destroy_texture(
    texture* texture
);