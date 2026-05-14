#pragma once

#include "renderer_types.inl"

struct static_mesh_data;
struct platform_state;

b8 renderer_frontend_initialize(u64 *memory_requirement, void *state, const char *application_name, struct platform_state *plat_state);

void renderer_frontend_shutdown();

b8 renderer_frontend_on_resized(u16 width, u16 height);

b8 renderer_frontend_draw_frame(render_packet *packet);

// HACK: should net be called outside of the engine
EAPI void renderer_frontend_set_view(mat4 view);

void renderer_frontend_create_texture(
    const u8 *pixels,
    texture *out_texture);

void renderer_frontend_destroy_texture(
    texture* texture
);

b8 renderer_frontend_create_material(material *material);

void renderer_frontend_destroy_material(material* material);