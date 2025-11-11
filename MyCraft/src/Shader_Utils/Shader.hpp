#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "stb_image.h"

class Shader {
    public:
    //---------------------
    // Initialization
    //---------------------
    void Init_Shader(GLuint& ShaderProgram, GLuint& Gui_ShaderProgram, GLuint& SelectionBox_Shader);

    GLuint Create_Shader(const std::string& vertex, const std::string& fragment);

    void Load_Texture(unsigned int &Texture_ID, GLenum TextureUnit);

    std::string LoadShaderSource(const std::string& path);

    //---------------------
    // Uniforms
    //---------------------
    void Set_Int(GLuint &Shader, const std::string &Name, int Value) const {
        glUniform1i(glGetUniformLocation(Shader, Name.c_str()), Value);
    }
    void Set_Float(GLuint &Shader, const std::string &Name, float Value) const {
        glUniform1f(glGetUniformLocation(Shader, Name.c_str()), Value);
    }
    void Set_Vec3(GLuint &Shader, const std::string &Name, glm::vec3 Value) const {
        glUniform3f(glGetUniformLocation(Shader, Name.c_str()), Value.x, Value.y, Value.z);
    }
    void Set_Mat4(GLuint &Shader, const std::string &Name, glm::mat4 Value) const {
        glUniformMatrix4fv(glGetUniformLocation(Shader, Name.c_str()), 1, GL_FALSE, &Value[0][0]);
    }
};