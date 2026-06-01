#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "memory/ememory.h"
#include "math/emath.h"

#include "resources/resource_types.h"

#include "systems/texture_system.h"
#include "systems/material_system.h"

// TODO temporary
#include "core/estring.h"
#include "core/event.h"

// TODO end temporary

typedef struct renderer_system_state
{
    renderer_backend backend;
    mat4 projection;
    mat4 view;
    f32 near_clip;
    f32 far_clip;

    texture default_texture;

} renderer_system_state;

static renderer_system_state *state_ptr = 0;

b8 renderer_frontend_initialize(u64 *memory_requirement, void *state, const char *application_name, struct platform_state *plat_state)
{
    if (memory_requirement && !state)
    {
        *memory_requirement = sizeof(renderer_system_state);
        return true;
    }

    state_ptr = state;
    ezero_out(state_ptr, sizeof(renderer_system_state));

    state_ptr->near_clip = 0.1f;
    state_ptr->far_clip = 1000.0f;

    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, plat_state, &state_ptr->backend);

    if (!state_ptr->backend.initialize(&state_ptr->backend, application_name, plat_state))
    {
        EN_FATAL("Failed to initialize renderer backend. Shutting down.");
        return false;
    }

    state_ptr->projection = mat4_perspective(
        45.0f,
        1920.0f / 1080.0f, // TODO do not hardcode
        state_ptr->near_clip,
        state_ptr->far_clip);
    state_ptr->view = mat4_identity();

    return true;
}

void renderer_frontend_shutdown()
{
    state_ptr->backend.shutdown(&state_ptr->backend);
    state_ptr = 0;
}

b8 renderer_frontend_on_resized(u16 width, u16 height)
{
    if (state_ptr)
    {
        state_ptr->projection = mat4_perspective(
            45.0f,
            (f32)width / (f32)height, // TODO do not hardcode
            state_ptr->near_clip,
            state_ptr->far_clip);
        state_ptr->backend.resized(&state_ptr->backend, width, height);
    }
    else
    {
        EN_ERROR("Renderer backend failed to resize. State ptr invalid.");
        return false;
    }
    return true;
}

b8 renderer_frontend_draw_frame(render_packet *packet)
{
    if (state_ptr->backend.begin_frame(&state_ptr->backend, packet->delta_time))
    {
        state_ptr->backend.update_global_state(state_ptr->projection, state_ptr->view, vec3_zero(), vec4_one(), 0);

        u32 count = packet->geometry_count;
        for (u32 i = 0; i < count; i++)
        {
            state_ptr->backend.draw_geometry(packet->geometries[i]);
        }

        b8 result = state_ptr->backend.end_frame(&state_ptr->backend, packet->delta_time);

        if (!result)
        {
            return false;
        }
    }
    return true;
}

EAPI void renderer_frontend_set_view(mat4 view)
{
    state_ptr->view = view;
}

void renderer_frontend_create_texture(
    const u8 *pixels,
    struct texture *texture)
{
    state_ptr->backend.create_texture(
        pixels,
        texture);
}

void renderer_frontend_destroy_texture(struct texture *texture)
{
    state_ptr->backend.destroy_texture(texture);
}

b8 renderer_frontend_create_material(material *material)
{
    return state_ptr->backend.create_material(material);
}

void renderer_frontend_destroy_material(material *material)
{
    state_ptr->backend.destroy_material(material);
}

b8 renderer_frontend_create_geometry(
    geometry *geometry,
    u32 vertex_count,
    const vertex_3d *vertices,
    u32 index_count,
    const u32 *indices)
{
    return state_ptr->backend.create_geometry(geometry, vertex_count, vertices, index_count, indices);
}

void renderer_frontend_destroy_geometry(geometry *geometry)
{
    state_ptr->backend.destroy_geometry(geometry);
}