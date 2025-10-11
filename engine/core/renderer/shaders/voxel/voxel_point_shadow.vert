#version 330 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_model_matrix;
uniform mat4 u_cube_view_proj; // per-face view-proj

void main() {
    gl_Position = u_cube_view_proj * u_model_matrix * vec4(a_position, 1.0);
}
