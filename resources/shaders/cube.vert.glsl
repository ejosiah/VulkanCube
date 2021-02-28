#version 450 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform mvp {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(location = 0) smooth out vec3 vColor;

void main() {
    gl_Position = proj * view * model * position;
    vColor = color;
}
