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

typedef struct geometry_render_data {
	mat4 model;
	geometry* geometry;
} geometry_render_data;

typedef struct render_packet
{
	f32 delta_time;

	u32 geometry_count;
	geometry_render_data* geometries;
} render_packet;

// Per rendered object per frame
typedef struct material_uniform_object {
	vec4 diffuse_color;
	vec4 v_reserved0;
	vec4 v_reserved1;
	vec4 v_reserved2;
} material_uniform_object;

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
	void (*draw_geometry)(geometry_render_data data);

	void(*create_texture)(
		const u8* pixels,
		struct texture* texture);

	void(*destroy_texture)(struct texture* texture);

	b8 (*create_material)(material* material);
	void (*destroy_material)(material* material);

	b8(*create_geometry)(geometry* geometry, u32 vertex_count, const vertex_3d* vertices,u32 index_count, const u32* indices);
	void(*destroy_geometry)(geometry* geometry);

} renderer_backend;