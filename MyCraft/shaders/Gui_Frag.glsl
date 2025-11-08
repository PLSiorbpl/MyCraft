#version 330 core

in vec2 FragPos;
in vec2 TexCoord;
flat in uint Special;

out vec4 FragColor;

void main() {
    FragColor = vec4(vec3(1), 0.85);
}