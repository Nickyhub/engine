#version 450

// set: descriptor_set
layout(set = 0, binding = 0) uniform global_uniform_object {
    mat4 proj;
    mat4 view;
} global_ubo;

layout(push_constant) uniform constants {
    // 128 Bytes are guaranteed
    mat4 model; // 64 Bytes
} u_push_constants;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(location = 0) out int out_mode;
layout(location = 1) out struct dto {
    vec2 tex_coord;
} out_dto;

void main() {
    gl_Position = global_ubo.proj * global_ubo.view * u_push_constants.model * vec4(in_position, 1.0);
    out_dto.tex_coord = in_texcoord;
}
