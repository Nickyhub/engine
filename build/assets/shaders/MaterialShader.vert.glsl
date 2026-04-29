#version 450

// set: descriptor_set
layout(set = 0, binding = 0) uniform global_uniform_object {
    mat4 proj;
    mat4 view;
} global_ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 position;

void main() {
    gl_Position = global_ubo.proj * global_ubo.view * vec4(inPosition, 1.0f);
}
