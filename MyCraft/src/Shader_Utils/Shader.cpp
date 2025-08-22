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

    int Vsuccess;
    char VinfoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Vsuccess);
    if(!Vsuccess) {
        glGetShaderInfoLog(vertexShader, 512, NULL, VinfoLog);
        std::cerr << "VERTEX COMPILATION ERROR\n" << VinfoLog << std::endl;
    }

    int Fsuccess;
    char FinfoLog[512];
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Fsuccess);
    if(!Fsuccess) {
        glGetShaderInfoLog(FragmentShader, 512, NULL, FinfoLog);
        std::cerr << "FRAGMENT COMPILATION ERROR\n" << FinfoLog << std::endl;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    const int texLoc = glGetUniformLocation(ShaderProgram, "tex");
    glUseProgram(ShaderProgram);
    glUniform1i(texLoc, 0); // GL_TEXTURE0 = 0
}

