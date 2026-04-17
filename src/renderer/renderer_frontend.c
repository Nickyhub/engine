#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "memory/ememory.h"

static renderer_backend *backend = 0;

b8 renderer_frontend_initialize(const char* application_name, struct platform_state* plat_state)
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
    if(!backend->resized(backend, width, height)) {
        EN_ERROR("Renderer backend failed to resize.");
        return false;
    }
    return true;
}

b8 renderer_frontend_draw_frame(render_packet *packet)
{
    if (backend->begin_frame(backend, packet->delta_time))
    {
        b8 result = backend->end_frame(backend, packet->delta_time);

        if (!result)
        {
            return false;
        }
    }
    return true;
}