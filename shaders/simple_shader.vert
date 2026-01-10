#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(set = 0, binding = 0) uniform Global {
    mat4 proj;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
    vec3 color;
} push;

layout(location = 0) out vec3 fragColor;

void main() {
    vec4 pos = vec4(position, 0.0, 1.0);
    vec4 world = push.model * pos;
    gl_Position = ubo.proj * world;
    fragColor = push.color;
}
