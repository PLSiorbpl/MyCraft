#version 330 core
layout(location = 0) in ivec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in uint aNormal;
layout(location = 3) in vec3 aOverlay;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Proj;
uniform vec3 ChunkOffset;

// Remember out name (out vec3 FragPos;) needs to be same in Fragment (in vec3 FragPos;)
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out vec3 Overlay;

vec3 getNormal(uint dir) {
    if (dir == 0u) return vec3(-1, 0, 0);   // -X
    if (dir == 1u) return vec3(1, 0, 0);  // +X
    if (dir == 2u) return vec3(0, -1, 0);   // -Y
    if (dir == 3u) return vec3(0, 1, 0);  // +Y
    if (dir == 4u) return vec3(0, 0, -1);   // -Z
    return vec3(0, 0, 1);                 // +Z
}

void main() {
    mat4 MVP = (Proj * View * Model);

    vec3 Pos = vec3(aPos / 16.0) + ChunkOffset;

    TexCoord = aTexCoord;
    FragPos = Pos;
    Normal = mat3(transpose(inverse(Model))) * getNormal(aNormal);
    Overlay = aOverlay;
    gl_Position = MVP * vec4(Pos, 1.0);
}