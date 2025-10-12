#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
//layout(location = 3) in vec3 aVColor;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Proj;

// Remember out name (out vec3 FragPos;) needs to be same in Fragment (in vec3 FragPos;)
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
//out vec3 VColor;

void main() {
    mat4 MVP = (Proj * View * Model);

    TexCoord = aTexCoord;
    FragPos = aPos;
    Normal = mat3(transpose(inverse(Model))) * aNormal;
    gl_Position = MVP * vec4(aPos, 1.0);
    //VColor = aVColor;
}