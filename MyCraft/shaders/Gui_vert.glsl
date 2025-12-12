#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aTexCoord;
layout(location = 2) in uint aFlags;

out vec2 FragPos;
out vec3 TexCoord;
flat out uint Flags;

uniform mat4 Model;
uniform mat4 Projection;

void main() {
    FragPos = aPos;
    gl_Position = Projection * Model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    Flags = aFlags;
}