#version 330 core

// Fullscreen triangle without vertex buffers
// gl_VertexID generates three vertices covering the screen

out vec2 v_uv; // 0..1

void main() {
    // NDC positions for a fullscreen triangle
    // (-1,-1), (3,-1), (-1,3)
    vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    v_uv = gl_Position.xy * 0.5 + 0.5; // map NDC to [0,1]
}
