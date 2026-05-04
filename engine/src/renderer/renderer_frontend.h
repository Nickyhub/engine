#pragma once

#include "renderer_types.inl"

struct static_mesh_data;
struct platform_state;

b8 renderer_frontend_initialize(const char *application_name, struct platform_state *plat_state);

void renderer_frontend_shutdown();

b8 renderer_frontend_on_resized(u16 width, u16 height);

b8 renderer_frontend_draw_frame(render_packet *packet);

// HACK: should net be called outside of the engine
EAPI void renderer_frontend_set_view(mat4 view);

void renderer_frontend_create_texture(
    const char *name,
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8 *pixels,
    b8 has_transparency,
    texture *out_texture);

void renderer_frontend_destroy_texture(
    texture* texture
);