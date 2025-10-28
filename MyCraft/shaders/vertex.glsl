#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in uvec2 aTexCoord;
layout(location = 2) in uint aNormal;
//layout(location = 3) in vec3 aVColor;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Proj;

// Remember out name (out vec3 FragPos;) needs to be same in Fragment (in vec3 FragPos;)
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
//out vec3 VColor;

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

    TexCoord = (vec2(aTexCoord)+1)/256;
    FragPos = aPos;
    Normal = mat3(transpose(inverse(Model))) * getNormal(aNormal);
    gl_Position = MVP * vec4(aPos, 1.0);
}