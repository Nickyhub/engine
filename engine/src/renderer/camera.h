#pragma once

#include "defines.h"
#include "math/math_types.h"

typedef struct camera {
    vec3 position;
    vec3 direction;

    vec3 up;
    vec3 right;
    mat4 view;
    
    f32 yaw;
    f32 pitch;

    f32 speed;
    b8 is_dirty;

    f32 sensitivity;
} camera;

EAPI void camera_create(camera* out_camera);
EAPI void camera_forward(camera* c, f32 delta_time);
EAPI void camera_left(camera* c, f32 delta_time);
EAPI void camera_right(camera* c, f32 delta_time);
EAPI void camera_backwards(camera* c, f32 delta_time);

EAPI void camera_yaw(camera* c, f32 amount, f32 delta_time);
EAPI void camera_pitch(camera* c, f32 amount, f32 delta_time);

EAPI mat4 camera_get_view_matrix(camera* c);
