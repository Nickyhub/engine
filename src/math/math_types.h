#pragma once


typedef struct vec2 {
    float v1;
    float v2;
} vec2;

typedef struct vec3 {
    float v1;
    float v2;
    float v3;
} vec3;

typedef struct vec4 {
    float v1;
    float v2;
    float v3;
    float v4;
} vec4;

typedef struct mat4 {
    float v1[4];
    float v2[4];
    float v3[4];
    float v4[4];
} mat4;

typedef struct vertex_3d {
    vec3 position;
    vec3 color;
} vertex_3d;