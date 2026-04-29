#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "memory/ememory.h"
#include "math/emath.h"

static renderer_backend *backend = 0;

b8 renderer_frontend_initialize(const char *application_name, struct platform_state *plat_state)
{
    backend = eallocate(sizeof(renderer_backend), MEMORY_TYPE_SYSTEM_STATE);

    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, plat_state, backend);

    if (!backend->initialize(backend, application_name, plat_state))
    {
        EN_FATAL("Failed to initialize renderer backend. Shutting down.");
        return false;
    }
    return true;
}

void renderer_frontend_shutdown()
{
    backend->shutdown(backend);
    efree(backend, sizeof(renderer_backend), MEMORY_TYPE_SYSTEM_STATE);
}

b8 renderer_frontend_on_resized(u16 width, u16 height)
{
    if (!backend->resized(backend, width, height))
    {
        EN_ERROR("Renderer backend failed to resize.");
        return false;
    }
    return true;
}

b8 renderer_frontend_draw_frame(render_packet *packet)
{
    if (backend->begin_frame(backend, packet->delta_time))
    {
        mat4 projection = mat4_perspective(
            45.0f,
            1920.0f / 1080.0f, // TODO do not hardcode
            0.1f,
            1000.0f);
        
        static f32 z = -3.0f;
        z -= 0.001f;
        mat4 view = mat4_translation((vec3){0.0f, 0.0f, z});
        
        vec4 color;
        color.x = 1.0f;
        color.y = 0.0f;
        color.z = 1.0f;
        color.w = 1.0f;
        
        backend->update_global_state(projection, view, vec3_zero(), vec4_one(), 0);
        b8 result = backend->end_frame(backend, packet->delta_time);

        if (!result)
        {
            return false;
        }
    }
    return true;
}