#version 450

// set: descriptor_set
layout(set = 0, binding = 0) uniform global_uniform_object {
    mat4 proj;
    mat4 view;
} global_ubo;

layout( push_constant ) uniform constants
{
    // 128 Bytes are guaranteed
	mat4 model; // 64 Bytes
} u_push_constants;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 position;

void main() {
    gl_Position = global_ubo.proj * global_ubo.view * u_push_constants.model * vec4(inPosition, 1.0f);
}
