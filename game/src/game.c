#include "game.h"

#include <core/logger.h>
#include <memory/ememory.h>

#include <core/input.h>
#include <math/emath.h>

// Dont include this from the engine if you have the camera
#include <renderer/renderer_frontend.h>

b8 game_mouse_moved(const void *sender, event_context context, event_type type);

b8 game_initialize(game *game_inst)
{
    game_state *state = game_inst->state;
    camera_create(&state->world_camera);

    EN_DEBUG("game_initialize() called. ");
    return true;
}

b8 game_update(game *game_inst, f32 delta_time)
{
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();

    game_state *state = game_inst->state;

    if (input_is_key_pressed('A'))
    {
        camera_left(&state->world_camera, delta_time);
    }

    if (input_is_key_pressed('D'))
    {
        camera_right(&state->world_camera, delta_time);
    }

    if (input_is_key_pressed('S'))
    {
        camera_backwards(&state->world_camera, delta_time);
    }

    if (input_is_key_pressed('W'))
    {
        camera_forward(&state->world_camera, delta_time);
    }

    if (input_is_key_pressed('Q'))
    {
        camera_yaw(&state->world_camera, -80.0f, delta_time);
    }

    if (input_is_key_pressed('E'))
    {
        camera_yaw(&state->world_camera, 80.0f, delta_time);
    }

    //    TODO CAMERA NOT WORKING WITH MOUSE NUTTE
    // vec2_u mouse_pos = input_get_latest_mouse_pos();
    // if (mouse_pos.x != state->last_mouse_x || mouse_pos.y != state->last_mouse_y)
    // {
    //     u32 x_offset = mouse_pos.x - state->last_mouse_x;
    //     u32 y_offset = mouse_pos.y - state->last_mouse_y;

    //     camera_pitch(&state->world_camera, (f32)y_offset * 0.2f, delta_time);
    //     camera_yaw(&state->world_camera, (f32)x_offset * 0.2f, delta_time);

    //     state->last_mouse_x = mouse_pos.x;
    //     state->last_mouse_y = mouse_pos.y;
    // }

    // HACK: dont call from outside the engine
    renderer_frontend_set_view(camera_get_view_matrix(&state->world_camera));
    // renderer_frontend_set_view(mat4_translation((vec3){0.0f, 0.0f, -20.0}));
}

b8 game_render(game *game_inst, f32 delta_time)
{
}

void game_on_resize(game *game_inst, u32 width, u32 height)
{
}
