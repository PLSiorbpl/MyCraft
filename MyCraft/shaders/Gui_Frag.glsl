#version 330 core

in vec2 FragPos;
in vec3 TexCoord;
flat in uint Flags;

out vec4 FragColor;

uniform sampler2D BaseTexture;

bool Get(uint flags, int bit) {
    return (flags & (1u << bit)) != 0u;
}

void main() {
    vec4 color;
    if (Get(Flags, 0)) {
        color = texture(BaseTexture, TexCoord.xy);
    } else {
        color = vec4(TexCoord, 0.85);
    }
    FragColor = vec4(color);
}