#pragma once
#include "defines.h"

typedef struct vec2 {
    f32 x;
    f32 y;
} vec2;

typedef struct vec2_u {
    u32 x;
    u32 y;
} vec2_u;

typedef struct vec3 {
    f32 x;
    f32 y;
    f32 z;
} vec3;

typedef struct vec4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} vec4;

typedef struct mat4 {
    f32 data[4][4];
} mat4;

typedef struct vertex_3d {
    vec3 position;
} vertex_3d;

typedef vec4 quat;