#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Proj;

out vec2 TexCoord;
out vec3 fragPos;
out vec3 Normal;

void main() {
    mat4 MVP = (Proj * View * Model);

    TexCoord = aTexCoord;
    fragPos = aPos;//vec3(Model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(Model))) * aNormal;
    gl_Position = MVP * vec4(aPos, 1.0);
}