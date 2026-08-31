#version 330 core

in vec2 FragPos;
in vec4 TexCoord;
flat in uint Flags;
in vec4 Color;

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
            float tex = texture2D(FontTexture, TexCoord.xy).a;
            color = vec4(Color.rgb, tex * Color.a);
        }
    } else {
        color = vec4(TexCoord);
    }
    FragColor = color;
}