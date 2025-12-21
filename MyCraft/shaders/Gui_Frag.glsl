#version 330 core

in vec2 FragPos;
in vec3 TexCoord;
flat in uint Flags;

out vec4 FragColor;

uniform sampler2D BaseTexture;
uniform sampler2D GuiTexture;
uniform sampler2D FontTexture;

bool Get(uint flags, int bit) {
    return (flags & (1u << bit)) != 0u;
}

void main() {
    vec4 color;
    if (Get(Flags, 0)) {
        if (!Get(Flags, 1) && !Get(Flags, 2)) {
            // Blocks
            color = texture(BaseTexture, TexCoord.xy);
        } else if (Get(Flags, 1)) {
            // Gui
            color = texture(GuiTexture, TexCoord.xy);
        } else if (Get(Flags, 2)) {
            // Font
            color = texture(FontTexture, TexCoord.xy);
            if (color.a > 0) {
                color.rgb = vec3(0.9647f, 0.9569f, 0.9255f);
            }
        }
    } else {
        color = vec4(TexCoord, 0.85);
    }
    FragColor = color;
}