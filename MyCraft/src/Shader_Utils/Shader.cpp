#include "Shader.hpp"

void Shader::Load_Texture(unsigned int &Texture_ID) {
    glGenTextures(1, &Texture_ID);
    glBindTexture(GL_TEXTURE_2D, Texture_ID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("MyCraft/Assets/Atlas.png", &width, &height, &nrChannels, 4);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture (skill Issue)" << std::endl;
    }
    stbi_image_free(data);
}

std::string Shader::LoadShaderSource(const std::string& path) {
    const std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cant Open File: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Shader::Init_Shader(GLuint &VAO, GLuint &VBO, GLuint &ShaderProgram) {
    unsigned int TextureID;
    Load_Texture(TextureID);

    const std::string vertexCode = LoadShaderSource("MyCraft/shaders/vertex.glsl");
    const std::string fragmentCode = LoadShaderSource("MyCraft/shaders/fragment.glsl");

    const char* vertexSrc = vertexCode.c_str();
    const char* fragmentSrc = fragmentCode.c_str();

    // Compilation of Shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(FragmentShader);
    
    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, vertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(FragmentShader);

    //glGenVertexArrays(1, &VAO);
    //glGenBuffers(1, &VBO);
//
    //glBindVertexArray(VAO);
//
    //glBindBuffer(GL_ARRAY_BUFFER, VBO);
//
    //glBufferData(GL_ARRAY_BUFFER, 1024 * 1024 * Vram, nullptr, GL_DYNAMIC_DRAW);
//
    //// aPos (location = 0)
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
//
    //// atexture (location = 1)
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
//
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    const int texLoc = glGetUniformLocation(ShaderProgram, "tex");
    glUseProgram(ShaderProgram);
    glUniform1i(texLoc, 0); // GL_TEXTURE0 = 0
}

