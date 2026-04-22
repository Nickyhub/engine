#version 450

layout(location = 0) in vec3 position; 
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(position.r, position.g, position.b, 1.0f);
}
