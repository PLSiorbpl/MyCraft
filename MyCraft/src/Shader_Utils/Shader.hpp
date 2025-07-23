#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <iostream>
#include <sstream>
#include <fstream>

class Shader {
    public:

    void Init_Shader(int Vram, GLuint &VAO, GLuint &VBO, GLuint &ShaderProgram);

    //void Set_Vec3();

    void Load_Texture(unsigned int &Texture_ID);

    std::string LoadShaderSource(const std::string& path);
};