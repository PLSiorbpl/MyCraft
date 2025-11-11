#include "Shader.hpp"

void Shader::Load_Texture(unsigned int &Texture_ID, GLenum TextureUnit) {
    glGenTextures(1, &Texture_ID);
    glBindTexture(GL_TEXTURE_2D, Texture_ID);
    //stbi_set_flip_vertically_on_load(true);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("MyCraft/Assets/Atlas.png", &width, &height, &nrChannels, 4);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture (skill Issue)" << std::endl;
    }
    stbi_image_free(data);

    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, Texture_ID);
}

std::string Shader::LoadShaderSource(const std::string& path) {
    const std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cant Open File: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::Create_Shader(const std::string& vertex, const std::string& fragment) {
    //---------------
    // Compiling
    //---------------
    std::string vertexStr = LoadShaderSource(vertex);
    const char* vertexSrc = vertexStr.c_str();
    
    std::string fragmentStr = LoadShaderSource(fragment);
    const char* fragmentSrc = fragmentStr.c_str();


    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    //---------------
    // Linking
    //---------------
    GLuint ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, vertexShader);
    glAttachShader(ShaderProgram, fragmentShader);
    glLinkProgram(ShaderProgram);

    //---------------
    // Error
    //---------------
    int Success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Success);
    if(!Success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "VERTEX COMPILATION ERROR\n" << infoLog << std::endl;
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Success);
    if(!Success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "FRAGMENT COMPILATION ERROR\n" << infoLog << std::endl;
    }

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if(!Success) {
        glGetProgramInfoLog(ShaderProgram, 512, NULL, infoLog);
        std::cerr << "SHADER LINKING ERROR\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return ShaderProgram;
}

void Shader::Init_Shader(GLuint& ShaderProgram, GLuint& Gui_ShaderProgram, GLuint& SelectionBox_Shader) {
    unsigned int TextureID;
    Load_Texture(TextureID, GL_TEXTURE0);

    ShaderProgram = Create_Shader("MyCraft/shaders/vertex.glsl", "MyCraft/shaders/fragment.glsl");
    Gui_ShaderProgram = Create_Shader("MyCraft/shaders/Gui_vert.glsl", "MyCraft/shaders/Gui_Frag.glsl");
    SelectionBox_Shader = Create_Shader("MyCraft/shaders/Selection_Vert.glsl", "MyCraft/shaders/Selection_Frag.glsl");

    glUseProgram(ShaderProgram);
    Set_Int(ShaderProgram, "BaseTexture", 0);
    glUseProgram(Gui_ShaderProgram);
    Set_Int(Gui_ShaderProgram, "BaseTexture", 0);
}