#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "memory/ememory.h"
#include "math/emath.h"

#include "resources/resource_types.h"

#include "systems/texture_system.h"

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

    // TODO temporary
    texture* test_diffuse;
    // TODO end temporary
} renderer_system_state;

static renderer_system_state *state_ptr = 0;

// TODO temp
b8 event_on_debug_event(const void *sender, event_context context, event_type type)
{
    const char *names[3] = {
        "galaxy1",
        "galaxy2",
        "galaxy3",
    };
    static i8 choice = 0;
    const char *old_name = names[choice];
    choice++;
    choice %= 3;

    // Acquire the new texture.
    state_ptr->test_diffuse = texture_system_acquire(names[choice], true);
    texture_system_release(old_name);
    return true;
}

// TODO end temp

b8 renderer_frontend_initialize(u64 *memory_requirement, void *state, const char *application_name, struct platform_state *plat_state)
{
    if (memory_requirement && !state)
    {
        *memory_requirement = sizeof(renderer_system_state);
        return true;
    }

    state_ptr = state;
    ezero_out(state_ptr, sizeof(renderer_system_state));
    // TODO temp
    registered_event e;
    e.callback = event_on_debug_event;
    e.type = EVENT_TYPE_DEBUG0;
    event_system_register_event(e);
    // TODO end temp

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

        mat4 model = mat4_translation((vec3){0.0f, 0.0f, 0.0f});
        // static f32 angle = 0.1f;
        // angle += 0.1f;
        // model = mat4_rotate_z(model, angle);
        geometry_render_data data = {0};
        data.model = model;
        data.object_id = 0;

        if (!state_ptr->test_diffuse)
        {
            state_ptr->test_diffuse = texture_system_get_default_texture();
        }

        data.textures[0] = state_ptr->test_diffuse;

        state_ptr->backend.update_object(data);
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
    const char *name,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8 *pixels,
    b8 has_transparency,
    struct texture *out_texture)
{
    state_ptr->backend.create_texture(
        name,
        width,
        height,
        channel_count,
        pixels,
        has_transparency,
        out_texture);
}

void renderer_frontend_destroy_texture(struct texture *texture)
{
    state_ptr->backend.destroy_texture(texture);
}