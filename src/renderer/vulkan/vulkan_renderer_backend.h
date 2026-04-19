#pragma once

#include <windows.h>

#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"
#include "vulkan_swapchain.h"

#include "core/event.h"

b8 vulkan_renderer_backend_initialize(
	renderer_backend *backend,
	const char *application_name,
	struct platform_state *plat_state);

b8 vulkan_renderer_backend_begin_frame(struct renderer_backend* backend, f32 delta_time);

b8 vulkan_renderer_backend_end_frame(struct renderer_backend* backend, f32 delta_time);

b8 vulkan_renderer_backend_on_resize(struct renderer_backend* backend, u16 width, u16 height);
void vulkan_renderer_backend_shutdown(struct renderer_backend* backend);
