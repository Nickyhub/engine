#pragma once

#include "math/math_types.h"
#include "containers/darray.h"

typedef struct renderer_frontend {
	u32 width;
	u32 height;
	
} renderer_frontend;

typedef struct renderer_backend {
	struct platform_state* plat_state;

	b8(*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
	void(*shutdown)(struct renderer_backend* backend);
	b8(*resized)(struct renderer_backend* backend, u16 width, u16 height);
	b8(*begin_frame)(struct renderer_backend* backend, f32 delta_time);
	b8(*end_frame)(struct renderer_backend* backend, f32 delta_time);
	void(*update_global_state)(mat4 projection, mat4 view, vec3 position, vec4 ambient_colour, i32 mode);
} renderer_backend;

typedef enum renderer_backend_type {
	RENDERER_BACKEND_TYPE_VULKAN,
	RENDERER_BACKEND_TYPE_OPENGL,
} renderer_backend_type;

typedef struct render_packet {
	f32 delta_time;
} render_packet;

typedef struct global_uniform_object {
	mat4 proj;
	mat4 view;
	mat4 m_reserved0; // memory alignment
	mat4 m_reserved1; // memory alignment
} global_uniform_object;
