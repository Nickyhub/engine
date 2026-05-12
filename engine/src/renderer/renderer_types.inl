#pragma once

#include "math/math_types.h"
#include "containers/darray.h"
#include "resources/resource_types.h"

typedef struct renderer_frontend
{
	u32 width;
	u32 height;
} renderer_frontend;

typedef enum renderer_backend_type
{
	RENDERER_BACKEND_TYPE_VULKAN,
	RENDERER_BACKEND_TYPE_OPENGL,
} renderer_backend_type;

typedef struct render_packet
{
	f32 delta_time;
} render_packet;

typedef struct geometry_render_data {
	u32 object_id;
	mat4 model;
	texture* textures[16];
} geometry_render_data;

// Per rendered object per frame
typedef struct object_uniform_object {
	vec4 diffuse_color;
	vec4 v_reserved0;
	vec4 v_reserved1;
	vec4 v_reserved2;
} object_uniform_object;

typedef struct global_uniform_object
{
	mat4 proj;
	mat4 view;
	mat4 m_reserved0; // memory alignment
	mat4 m_reserved1; // memory alignment
} global_uniform_object;

typedef struct renderer_backend
{
	struct platform_state *plat_state;
	// Pointers to default textures.

	b8 (*initialize)(struct renderer_backend *backend, const char *application_name, struct platform_state *plat_state);
	void (*shutdown)(struct renderer_backend *backend);
	b8 (*resized)(struct renderer_backend *backend, u16 width, u16 height);
	b8 (*begin_frame)(struct renderer_backend *backend, f32 delta_time);
	b8 (*end_frame)(struct renderer_backend *backend, f32 delta_time);
	void (*update_global_state)(mat4 projection, mat4 view, vec3 position, vec4 ambient_colour, i32 mode);
	void (*update_object)(geometry_render_data data);

	void(*create_texture)(
		const char* name,
		i32 width,
		i32 height,
		i32 channel_count,
		const u8* pixels,
		b8 has_transparency,
		struct texture* out_texture);

	void(*destroy_texture)(struct texture* texture);

} renderer_backend;