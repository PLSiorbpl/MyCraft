#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "stb_image.h"
#include "Globals.hpp"

class Shader {
    public:
    //---------------------
    // Initialization
    //---------------------
    static void Init_Shader();

    static GLuint Create_Shader(const std::string& vertex, const std::string& fragment);

    static unsigned int Load_Texture(const std::string& path, GLenum TextureUnit);

    static std::string LoadShaderSource(const std::string& path);

    //---------------------
    // Uniforms
    //---------------------
    static void Set_Int(const GLuint &Shader, const std::string &Name, const int Value) {
        glUniform1i(glGetUniformLocation(Shader, Name.c_str()), Value);
    }

    static void Set_Float(const GLuint &Shader, const std::string &Name, const float Value) {
        glUniform1f(glGetUniformLocation(Shader, Name.c_str()), Value);
    }

    static void Set_Vec3(const GLuint &Shader, const std::string &Name, const glm::vec3 Value) {
        glUniform3f(glGetUniformLocation(Shader, Name.c_str()), Value.x, Value.y, Value.z);
    }

    static void Set_Mat4(const GLuint &Shader, const std::string &Name, glm::mat4 Value) {
        glUniformMatrix4fv(glGetUniformLocation(Shader, Name.c_str()), 1, GL_FALSE, &Value[0][0]);
    }
};