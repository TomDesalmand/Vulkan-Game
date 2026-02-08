#version 450

// Vertex attributes for simple geometry rendering
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

// Global uniform buffer for projection matrix
layout(set = 0, binding = 0) uniform Global {
    mat4 proj;
} ubo;

// Push constants for per-draw data
layout(push_constant) uniform Push {
    mat4 model;
    vec3 tintColor;
} push;

// Output to fragment shader
layout(location = 0) out vec3 fragColor;

void main() {
    // Transform position to clip space
    vec4 worldPos = push.model * vec4(position, 0.0, 1.0);
    gl_Position = ubo.proj * worldPos;

    // Combine vertex color with push constant tint
    fragColor = color * push.tintColor;
}
