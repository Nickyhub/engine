#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "memory/ememory.h"
#include "math/emath.h"

#include "resources/resource_types.h"

typedef struct renderer_system_state
{
    renderer_backend backend;
    mat4 projection;
    mat4 view;
    f32 near_clip;
    f32 far_clip;

    texture default_texture;
} renderer_system_state;

static renderer_system_state *state = 0;

b8 renderer_frontend_initialize(const char *application_name, struct platform_state *plat_state)
{
    state = eallocate(sizeof(renderer_system_state), MEMORY_TYPE_SYSTEM_STATE);
    ezero_out(state, sizeof(renderer_system_state));

    state->near_clip = 0.1f;
    state->far_clip = 1000.0f;
    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, plat_state, &state->backend);

    if (!state->backend.initialize(&state->backend, application_name, plat_state))
    {
        EN_FATAL("Failed to initialize renderer backend. Shutting down.");
        return false;
    }

    state->projection = mat4_perspective(
        45.0f,
        1920.0f / 1080.0f, // TODO do not hardcode
        state->near_clip,
        state->far_clip);
    state->view = mat4_identity();

    EN_TRACE("Creating default texture...");
    const u32 tex_dimension = 256;
    const u32 channels = 4;
    const u32 pixel_count = tex_dimension * tex_dimension;

    u8 pixels[pixel_count * channels];
    eset_memory(pixels, sizeof(u8) * pixel_count * channels, 255);

    // Each pixel
    for (u32 row = 0; row < tex_dimension; row++)
    {
        for (u32 col = 0; col < tex_dimension; col++)
        {
            u32 index = (row * tex_dimension) + col;
            u32 index_bpp = index * channels;
            if (row % 2)
            {
                if (col % 2)
                {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            }
            else
            {
                if (!(col % 2))
                {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            }
        }
    }

    renderer_frontend_create_texture(
        "default",
        false,
        256,
        256,
        4,
        pixels,
        false,
        &state->default_texture);

    return true;
}

void renderer_frontend_shutdown()
{
    if(state) {
        renderer_frontend_destroy_texture(&state->default_texture);
    }

    state->backend.shutdown(&state->backend);
    efree(state, sizeof(renderer_system_state), MEMORY_TYPE_SYSTEM_STATE);
}

b8 renderer_frontend_on_resized(u16 width, u16 height)
{
    if (state)
    {
        state->projection = mat4_perspective(
            45.0f,
            (f32)width / (f32)height, // TODO do not hardcode
            state->near_clip,
            state->far_clip);
        state->backend.resized(&state->backend, width, height);
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
    if (state->backend.begin_frame(&state->backend, packet->delta_time))
    {
        state->backend.update_global_state(state->projection, state->view, vec3_zero(), vec4_one(), 0);

        mat4 model = mat4_translation((vec3){0.0f, 0.0f, 0.0f});
        static f32 angle = 0.1f;
        angle += 0.1f;
        model = mat4_rotate_z(model, angle);
        geometry_render_data data = {0};
        data.model = model;
        data.object_id = 0;
        data.textures[0] = &state->default_texture;

        state->backend.update_object(data);
        b8 result = state->backend.end_frame(&state->backend, packet->delta_time);

        if (!result)
        {
            return false;
        }
    }
    return true;
}

EAPI void renderer_frontend_set_view(mat4 view)
{
    state->view = view;
}

void renderer_frontend_create_texture(
    const char *name,
    b8 auto_release,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8 *pixels,
    b8 has_transparency,
    struct texture *out_texture)
{
    state->backend.create_texture(
        name,
        auto_release,
        width,
        height,
        channel_count,
        pixels,
        has_transparency,
        out_texture);
}

void renderer_frontend_destroy_texture(struct texture *texture)
{
    state->backend.destroy_texture(texture);
}