#pragma once


typedef struct vec2 {
    float x;
    float y;
} vec2;

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef struct vec4 {
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct mat4 {
    float v1[4];
    float v2[4];
    float v3[4];
    float v4[4];
} mat4;

typedef struct vertex_3d {
    vec3 position;
} vertex_3d;