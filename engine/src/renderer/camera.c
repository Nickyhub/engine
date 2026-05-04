#include "camera.h"
#include "math/emath.h"

EAPI void camera_create(camera *out_camera)
{
    out_camera->direction = (vec3){0.0f, 0.0f, -1.0f};
    out_camera->position = (vec3){0.0f, 0.0f, -5.0f};
    out_camera->up = (vec3){0.0f, 1.0f, 0.0f};
    out_camera->right = vec3_cross(out_camera->direction, out_camera->up);

    out_camera->sensitivity = 0.1f;
    out_camera->yaw = -90.0f;
    out_camera->pitch = 0.0f;

    out_camera->is_dirty = true; // first get ob view matrix needs so be correct
    out_camera->speed = 2.0f;
    out_camera->view = mat4_look_at(
        out_camera->position,
        vec3_add(out_camera->position, out_camera->direction),
        out_camera->up);
}

EAPI void camera_forward(camera *c, f32 delta_time)
{
    f32 speed = -delta_time * c->speed;
    c->position = vec3_add(
        c->position,
        vec3_scale(speed, c->direction));
    c->is_dirty = true;
}

EAPI void camera_left(camera *c, f32 delta_time)
{
    c->position = vec3_add(c->position, vec3_scale(-delta_time * c->speed, c->right));
    c->is_dirty = true;
}

EAPI void camera_right(camera *c, f32 delta_time)
{
    c->position = vec3_add(c->position, vec3_scale(delta_time * c->speed, c->right));
    c->is_dirty = true;
}
EAPI void camera_backwards(camera *c, f32 delta_time)
{
    f32 speed = delta_time * c->speed;
    c->position = vec3_add(
        c->position,
        vec3_scale(speed, c->direction));
    c->is_dirty = true;
}

EAPI mat4 camera_get_view_matrix(camera *c)
{
    if (c->is_dirty)
    {
        // Update direction
        c->direction.x = ecos(deg_to_rad(c->yaw)) * ecos(deg_to_rad(c->pitch));
        c->direction.y = esin(deg_to_rad(c->pitch));
        c->direction.z = esin(deg_to_rad(c->yaw)) * ecos(deg_to_rad(c->pitch));

        c->right = vec3_normalized(vec3_cross(c->direction, (vec3){0.0f, 1.0f, 0.0f}));
        c->up = vec3_normalized(vec3_cross(c->right, c->direction));

        c->view = mat4_look_at(c->position, vec3_add(c->position, c->direction), c->up);
    }
    c->is_dirty = false;
    return c->view;
}

EAPI void camera_yaw(camera *c, f32 amount, f32 delta_time)
{
    c->yaw += amount * delta_time * c->sensitivity;
    c->is_dirty = true;
}

EAPI void camera_pitch(camera *c, f32 amount, f32 delta_time)
{
    c->pitch += amount * delta_time * c->sensitivity;
    c->pitch = EN_CLAMP(-89.0f, 89.0f, c->pitch);
    c->is_dirty = true;
}