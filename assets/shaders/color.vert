#version 330 core

// Model view projection matrix
uniform mat4 u_MVP;
uniform vec3 u_color;

layout(location = 0) in vec4 position;
out vec3 v_color; // output a tex coord to the fragment shader

void main() {
    gl_Position = u_MVP * position;
    v_color = u_color;
}