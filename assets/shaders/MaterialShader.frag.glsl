#version 450

layout(location = 0) in vec3 position;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform local_uniform_object {
    vec4 diffuse_colour;
} object_ubo;

void main() {
    outColor = object_ubo.diffuse_colour;
}
