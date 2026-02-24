#version 330 core

out vec2 uv;

// Triangle covering entire screen
vec2 vertices[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

void main() {
    // idk why error but should work
    vec2 pos = vertices[gl_VertexID];
    uv = pos * 0.5 + 0.5;

    gl_Position = vec4(pos, 0.0, 1.0);
}