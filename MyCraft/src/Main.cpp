#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <cmath>
#include <algorithm>
#include <windows.h>
#include <psapi.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FastNoiseLite.h"
#include "Utils/FPS.hpp"
#include "Render/Camera.hpp"
#include "Utils/Settings.hpp"
#include "World/Block.hpp"
#include "World/Chunk.hpp"
#include "Player/Movement.hpp"

int CHUNK_WIDTH;
int CHUNK_HEIGHT;
int CHUNK_DEPTH;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct Game_Variables {
    int Render_Distance;
    int VRamAlloc; // In MB
    GLint sizeInBytes = 0;
    uint64_t Frame = 0;
    glm::ivec3 Last_Chunk;
    bool ChunkUpdated;
    float TickRate;
    float Tick_Timer = 0.0f;
    int V_Sync;
};

struct Entity {
    glm::vec3 Pos;
    glm::ivec3 Chunk;
    float Speed;
};

//struct Block {
//    int8_t id;
//    bool transparent; // glass
//    bool solid;       // Colisions
//    uint8_t light;    // How Block is Lighted
//
//    Block(uint8_t id = 0, bool transparent = false, bool solid = false, uint8_t light = 0)
//        : id(id), transparent(transparent), solid(solid), light(light) {}
//};

//class Chunk {
//private:
//    std::vector<Block> blocks;
//public:
//    static const std::map<uint8_t, Block> BlockDefs;
//
//    Chunk() : blocks(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH) {}
//
//    int index(int x, int y, int z) const {
//        //assert(x >= 0 && x < CHUNK_WIDTH);
//        //assert(y >= 0 && y < CHUNK_HEIGHT);
//        //assert(z >= 0 && z < CHUNK_DEPTH);
//        return x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH;
//    }
//
//    const Block& get(int x, int y, int z) const {
//        return blocks[index(x, y, z)];
//    }
//
//    void set(int x, int y, int z, const Block& block) {
//        blocks[index(x, y, z)] = block;
//    }
//
//    void setID(int x, int y, int z, uint8_t id) {
//        blocks[index(x, y, z)].id = id;
//    }
//};

//const std::map<uint8_t, Block> Chunk::BlockDefs = {
//    { 0, Block(0, false, false) }, // Air
//    { 1, Block(1, false, true)  }, // Stone
//    { 2, Block(2, false,  true)  }, // Grass
//};

void Get_World_Chunk(int Chunk_x, int Chunk_z, std::map<std::pair<int, int>, Chunk>& World) {
    Chunk Chunk;
    Chunk.width = CHUNK_WIDTH;
    Chunk.height = CHUNK_HEIGHT;
    Chunk.depth = CHUNK_DEPTH;
    Chunk.chunkX = Chunk_x;
    Chunk.chunkZ = Chunk_z;
    Chunk.blocks.resize(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH);
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    const float baseFreq = 0.1f;
    const float baseAmp = 0.30f;
    const int octaves = 4;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            float worldX = Chunk_x * CHUNK_WIDTH + x;
            float worldZ = Chunk_z * CHUNK_DEPTH + z;

            // Leyered noise
            float total = 0.0f;
            float freq = baseFreq;
            float amp = baseAmp;

            for (int i = 0; i < octaves; ++i) {
                total += noise.GetNoise(worldX * freq, worldZ * freq) * amp;
                freq *= 2.0f;
                amp *= 0.5f;
            }

            float height_f = total * 0.5f + 0.5f;
            int height = static_cast<int>(height_f * CHUNK_HEIGHT * 0.8f);
            for (int y = 0; y <= height && y < CHUNK_HEIGHT; ++y) {
                if (y < height - 1)
                    Chunk.set(x, y, z, Chunk::BlockDefs.at(1)); // Stone
                else
                    Chunk.set(x, y, z, Chunk::BlockDefs.at(2)); // Grass
            }
        }
    }

    World[{Chunk_x, Chunk_z}] = std::move(Chunk);
}

void Generate_Chunks(const Entity& Player, int Render_Dist, std::map<std::pair<int, int>, Chunk>& World, camera &Camera) {
    glm::ivec3 Start_Chunk = Player.Chunk;

    for (int dx = -Render_Dist; dx <= Render_Dist; ++dx) {
        for (int dz = -Render_Dist; dz <= Render_Dist; ++dz) {
            int chunkX = Start_Chunk.x + dx;
            int chunkZ = Start_Chunk.z + dz;

            std::pair<int, int> key = {chunkX, chunkZ};

            if (World.find(key) == World.end()) {
                Get_World_Chunk(chunkX, chunkZ, World);
            }
        }
    }
}

void Remove_Chunks_Outside_Radius(const glm::ivec3& PlayerChunk, int Render_Dist, std::map<std::pair<int, int>, Chunk>& World, camera &Camera, Entity &Player) {
    // Keys to Delete
    std::vector<std::pair<int,int>> toRemove;

    for (const auto& [key, chunk] : World) {
        int chunkX = key.first;
        int chunkZ = key.second;

        int dx = chunkX - PlayerChunk.x;
        int dz = chunkZ - PlayerChunk.z;

        int dist = std::max(std::abs(dx), std::abs(dz));

        if (dist > Render_Dist) { //|| !IsChunkInFrontOfCamera(Player.Pos, Camera.Yaw, Camera.Pitch, chunkX, chunkZ)) {
            toRemove.push_back(key);
        }
    }

    // Delete chunks
    for (const auto& key : toRemove) {
        World.erase(key);
    }
}

void AddCube(std::vector<float>& vertices, float wx, float wy, float wz, const Chunk& chunk, int Localx, int Localy, int Localz) {
    float size = 1.0f;

    glm::vec3 p000 = {wx,     wy,     wz};
    glm::vec3 p001 = {wx,     wy,     wz+size};
    glm::vec3 p010 = {wx,     wy+size, wz};
    glm::vec3 p011 = {wx,     wy+size, wz+size};
    glm::vec3 p100 = {wx+size, wy,     wz};
    glm::vec3 p101 = {wx+size, wy,     wz+size};
    glm::vec3 p110 = {wx+size, wy+size, wz};
    glm::vec3 p111 = {wx+size, wy+size, wz+size};

    int texX = 0;
    int texY = 0;

    if (chunk.get(Localx, Localy, Localz).id == 1) {
        texX = 0;
        texY = 0;
    } else if (chunk.get(Localx, Localy, Localz).id == 2) {
        texX = 1;
        texY = 0;
    } else {
        texX = 0;
        texY = 1;
    }

    float tileSize = 1.0f / 8.0f; // 0.125   8 textures in a row

    float u = texX * tileSize;
    float v = texY * tileSize;

    glm::vec2 uv00 = {u, v};                         // left down
    glm::vec2 uv10 = {u + tileSize, v};              // right up
    glm::vec2 uv01 = {u, v + tileSize};              // left up
    glm::vec2 uv11 = {u + tileSize, v + tileSize};   // right down

    // push trójkąt z 3 parami: pozycja i UV
    auto pushTri = [&](glm::vec3 a, glm::vec2 uva,
                       glm::vec3 b, glm::vec2 uvb,
                       glm::vec3 c, glm::vec2 uvc) {
        for (int i = 0; i < 3; ++i) {
            const glm::vec3& v = i == 0 ? a : (i == 1 ? b : c);
            const glm::vec2& uv = i == 0 ? uva : (i == 1 ? uvb : uvc);
            vertices.insert(vertices.end(), { v.x, v.y, v.z, uv.x, uv.y });
        }
    };

    // FRONT (z+)
    if (Localz + 1 >= CHUNK_DEPTH || chunk.get(Localx,Localy,Localz+1).id == 0) {
        pushTri(p001, uv00, p101, uv10, p111, uv11);
        pushTri(p001, uv00, p111, uv11, p011, uv01);
    }

    // BACK (z-)
    if ((Localz - 1 < 0) || chunk.get(Localx,Localy,Localz-1).id == 0) {
        pushTri(p100, uv00, p000, uv10, p010, uv11);
        pushTri(p100, uv00, p010, uv11, p110, uv01);
    }

    // LEFT (x-)
    if ((Localx - 1 < 0) || chunk.get(Localx-1,Localy,Localz).id == 0) {
        pushTri(p000, uv00, p001, uv10, p011, uv11);
        pushTri(p000, uv00, p011, uv11, p010, uv01);
    }

    // RIGHT (x+)
    if (Localx + 1 >= CHUNK_WIDTH || chunk.get(Localx+1,Localy,Localz).id == 0) {
        pushTri(p100, uv00, p101, uv10, p111, uv11);
        pushTri(p100, uv00, p111, uv11, p110, uv01);
    }

    // TOP (y+)
    if (Localy + 1 >= CHUNK_HEIGHT || chunk.get(Localx,Localy+1,Localz).id == 0) {
        pushTri(p010, uv00, p011, uv10, p111, uv11);
        pushTri(p010, uv00, p111, uv11, p110, uv01);
    }

    // BOTTOM (y-)
    if ((Localy - 1 < 0) || chunk.get(Localx,Localy-1,Localz).id == 0) {
        pushTri(p000, uv00, p100, uv10, p101, uv11);
        pushTri(p000, uv00, p101, uv11, p001, uv01);
    }
}

void GenerateMesh(const Chunk& chunk, std::vector<float>& outVertices, int chunkX, int chunkZ) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_DEPTH; ++z) {
                if (chunk.get(x, y, z).id != 0) { // 0 is Air
                    float wx = chunkX * CHUNK_WIDTH + x;
                    float wy = y;
                    float wz = chunkZ * CHUNK_DEPTH + z;

                    AddCube(outVertices, wx, wy, wz, chunk, x, y, z);
                }
            }
        }
    }
}

glm::mat4 GetViewMatrix(const camera& cam) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));
    direction.y = sin(glm::radians(cam.Pitch));
    direction.z = sin(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));

    glm::vec3 front = glm::normalize(direction);
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0,1,0)));
    glm::vec3 up    = glm::normalize(glm::cross(right, front));

    return glm::lookAt(cam.Position, cam.Position + front, up);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static camera* cam = reinterpret_cast<camera*>(glfwGetWindowUserPointer(window));

    if (cam->FirstMouse) {
        cam->LastX = xpos;
        cam->LastY = ypos;
        cam->FirstMouse = false;
    }

    float xoffset = xpos - cam->LastX;
    float yoffset = cam->LastY - ypos;

    cam->LastX = xpos;
    cam->LastY = ypos;

    xoffset *= cam->Sensitivity;
    yoffset *= cam->Sensitivity;

    cam->Yaw   += xoffset;
    cam->Pitch += yoffset;

    // clamp Pitch
    cam->Pitch = std::clamp(cam->Pitch, -89.0f, 89.0f);
}

//bool isSolidAt(glm::vec3 pos, const std::map<std::pair<int, int>, Chunk>& World) {
//    int blockX = static_cast<int>(floor(pos.x));
//    int blockY = static_cast<int>(floor(pos.y-1.8f));
//    int blockZ = static_cast<int>(floor(pos.z));
//
//    int chunkX = floor((float)blockX / CHUNK_WIDTH);
//    int chunkZ = floor((float)blockZ / CHUNK_DEPTH);
//
//    int localX = blockX - chunkX * CHUNK_WIDTH;
//    int localZ = blockZ - chunkZ * CHUNK_DEPTH;
//
//    if (localX < 0 || localX >= CHUNK_WIDTH || localZ < 0 || localZ >= CHUNK_DEPTH) {
//        return false;
//    }
//    
//    auto it = World.find({chunkX, chunkZ});
//    if (it != World.end()) {
//        const Chunk& chunk = it->second;
//
//        if (blockY >= 0 && blockY < CHUNK_HEIGHT) {
//            return chunk.get(localX, blockY, localZ).solid;
//        }
//    }
//
//    return false;
//}

//bool isSolidAround(glm::vec3 pos, const std::map<std::pair<int, int>, Chunk>& World, float margin = 0.25f, float height = 1.8f) {
//
//    for (float dx : {-margin, margin}) {
//        for (float dz : {-margin, margin}) {
//            for (float dy : {0.0f, height / 2.0f, height}) {
//                glm::vec3 offsetPos = pos + glm::vec3(dx, dy, dz);
//                if (isSolidAt(offsetPos, World)) {
//                    return true;
//                }
//            }
//        }
//    }
//    return false;
//}

void Input_Handler(camera &Camera, GLFWwindow* window, float deltaTime, std::map<std::pair<int, int>, Chunk>& World) {
    Movement movement;

    Camera.Position.x += 0.1f;
    movement.Init(Camera);
    movement.Input(window, Camera);
    movement.TestColisions(Camera, World, glm::vec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH));
    movement.Damp(Camera);
    //float velocity = Camera.Speed;
    //float Jump_Boost = Camera.JumpStrength;
    //glm::vec3 direction(0.0f);
    //glm::vec2 Cos;
    //glm::vec2 Sin;
    //Cos.x = cos(glm::radians(Camera.Pitch));
    //Cos.y = cos(glm::radians(Camera.Yaw));
    //Sin.x = sin(glm::radians(Camera.Pitch));
    //Sin.y = sin(glm::radians(Camera.Yaw));
//
    //// Sterowanie
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    //    Camera.Vel.x += -velocity * Sin.y;
    //    Camera.Vel.z +=  velocity * Cos.y;
    //}
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    //    Camera.Vel.x +=  velocity * Sin.y;
    //    Camera.Vel.z += -velocity * Cos.y;
    //}
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    //    Camera.Vel.x += -velocity * Cos.y;
    //    Camera.Vel.z += -velocity * Sin.y;
    //}
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    //    Camera.Vel.x +=  velocity * Cos.y;
    //    Camera.Vel.z +=  velocity * Sin.y;
    //}
//
    //Camera.Vel.y -= Camera.Gravity;
//
    //// Jump
    //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && Camera.onGround) {
    //    Camera.Vel.y = Jump_Boost;
    //    Camera.onGround = false;
    //}
    //if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    //    Camera.Vel.y = Jump_Boost;
    //}
//
    //if (glm::length(direction) > 0.0f) {
    //    direction = glm::normalize(direction);
    //    Camera.Vel.x += direction.x * velocity;
    //    Camera.Vel.z += direction.z * velocity;
    //}
//
    //Camera.Vel.x = std::clamp(Camera.Vel.x, -0.1f, 0.1f);
    //Camera.Vel.z = std::clamp(Camera.Vel.z, -0.1f, 0.1f);
    //Camera.Vel.y = std::clamp(Camera.Vel.y, -0.2f, 0.2f);
//
    //// get unstuck from block
    //if (isSolidAt(Camera.Position, World)) {
    //    Camera.Position.y += 1.0f;
    //}
//
    //glm::vec3 testPos;
//
    //testPos = Camera.Position + glm::vec3(Camera.Vel.x, 0, 0);
    //if (!isSolidAround(testPos, World)) {
    //    Camera.Position.x = testPos.x;
    //} else {
    //    Camera.Vel.x = 0.0f;
    //}
//
    //testPos = Camera.Position + glm::vec3(0, Camera.Vel.y, 0);
    //if (!isSolidAround(testPos, World)) {
    //    Camera.Position.y = testPos.y;
    //    Camera.onGround = false;
    //} else {
    //    if (Camera.Vel.y < 0.0f) {
    //        Camera.onGround = true;
    //    }
    //    Camera.Vel.y = 0.0f;
    //}
//
    //testPos = Camera.Position + glm::vec3(0, 0, Camera.Vel.z);
    //if (!isSolidAround(testPos, World)) {
    //    Camera.Position.z = testPos.z;
    //} else {
    //    Camera.Vel.z = 0.0f;
    //}
//
    //auto damp = [&](float& v) {
    //    if (v > Camera.CameraDrag)
    //        v -= Camera.CameraDrag;
    //    else if (v < -Camera.CameraDrag)
    //        v += Camera.CameraDrag;
    //    else
    //        v = 0;
    //};
    //damp(Camera.Vel.x);
    //damp(Camera.Vel.z);
}

void Load_Texture(unsigned int &Texture_ID) {
    glGenTextures(1, &Texture_ID);
    glBindTexture(GL_TEXTURE_2D, Texture_ID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("MyCraft/Assets/Stone.png", &width, &height, &nrChannels, 4);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture (skill Issue)" << std::endl;
    }
    stbi_image_free(data);
}

std::string LoadShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cant Open File: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

class Game {
private:
    SIZE_T ramUsed;
    PROCESS_MEMORY_COUNTERS meminfo;
    FPS Fps;
    camera Camera;
    Game_Variables game;
    GLuint VAO, VBO, ShaderProgram;
    std::vector<float> vertecies;
    Entity Player;
    std::map<std::pair<int, int>, Chunk> World;
    float DeltaTime;
    int width, height;

public:
    Settings_Loader Settings;
    GLFWwindow* window;

    bool Init_Window();
    void Init_Shader();
    void MainLoop();
    void CleanUp();
    void Init_Settings(const std::string Path);
};

void Game::Init_Settings(const std::string Path) {
    Settings.Load_Settings(Path);

    game.Render_Distance = Settings.GetInt("Render Distance");
    game.VRamAlloc = Settings.GetInt("VRam Alloc");
    CHUNK_WIDTH = Settings.GetInt("Chunk Width");
    CHUNK_HEIGHT = Settings.GetInt("Chunk Height");
    CHUNK_DEPTH = Settings.GetInt("Chunk Depth");
    game.V_Sync = Settings.GetInt("V-Sync");
    game.TickRate = 1.0f / Settings.GetFloat("Tick Rate");
}

void Game::CleanUp() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(ShaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Game::Init_Window() {
     if (!glfwInit()) {
        std::cerr << "Cant Initalize GLFW (skill issue)!\n";
        return true;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor); // Window or Full Screen

    width = mode->width;
    height = mode->height;

    window = glfwCreateWindow(width, height, "MyCraft", nullptr, nullptr);
    if (!window) {
        std::cerr << "skill issue with GLFW!\n";
        glfwTerminate();
        return true;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(game.V_Sync); // V-sync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Skill Issue of OpenGl and Glad!\n";
        return true;
    }

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, &Camera);
    glfwSetCursorPosCallback(window, mouse_callback);

    game.Last_Chunk = glm::ivec3(999, 999, 999);
    
    return 0;
}

void Game::Init_Shader() {
    unsigned int TextureID;
    Load_Texture(TextureID);
    // Load Shaders
    std::string vertexCode = LoadShaderSource("MyCraft/shaders/vertex.glsl");
    std::string fragmentCode = LoadShaderSource("MyCraft/shaders/fragment.glsl");

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

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Amount of VRAM Allocated in MB
    glBufferData(GL_ARRAY_BUFFER, 1024 * 1024 * game.VRamAlloc, nullptr, GL_DYNAMIC_DRAW);

    // aPos (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // atexture (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    int texLoc = glGetUniformLocation(ShaderProgram, "tex");
    glUseProgram(ShaderProgram);
    glUniform1i(texLoc, 0); // GL_TEXTURE0 = 0

} 

void Tick_Update(camera &Camera, GLFWwindow* window, float &DeltaTime, std::map<std::pair<int, int>, Chunk> &World) {
    Input_Handler(Camera, window, DeltaTime, World);
}

void Game::MainLoop() {
    glfwGetWindowSize(window, &width, &height);
    Fps.Init();
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
            
        // Main Engine -------------------------------------------------------------------
            
            DeltaTime = Fps.Start();
            game.Tick_Timer += DeltaTime;

            // Tick abdejt
            while (game.Tick_Timer >= game.TickRate) {
                game.Tick_Timer -= game.TickRate;
                if (!game.ChunkUpdated) {
                    Tick_Update(Camera, window, DeltaTime, World);
                }
            }

        // Clearing
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MVP
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = GetViewMatrix(Camera);
            glm::mat4 proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 600.0f);
            glm::mat4 MVP = proj * view * model;

            GLuint mvpLoc = glGetUniformLocation(ShaderProgram, "MVP");
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &MVP[0][0]);

        // Chunk Update
            Player.Pos = Camera.Position;
            Player.Chunk.x = static_cast<int>(std::floor(Camera.Position.x / CHUNK_WIDTH));
            Player.Chunk.y = 0;
            Player.Chunk.z = static_cast<int>(std::floor(Camera.Position.z / CHUNK_DEPTH));

        // Update Logic
            game.ChunkUpdated = false;
            if (Player.Chunk != game.Last_Chunk) {
                game.ChunkUpdated = true;
                game.Last_Chunk = Player.Chunk;
            }

        // Generating Chunks
            if (game.ChunkUpdated) {
                Generate_Chunks(Player, game.Render_Distance, World, Camera);
                Remove_Chunks_Outside_Radius(Player.Chunk, game.Render_Distance, World, Camera, Player);
                // Generating Mesh
                vertecies.clear();
                for (auto& [key, chunk] : World) {
                    GenerateMesh(chunk, vertecies, key.first, key.second);
                }
            }

        // Draw On Screen & send to GPU
            if (!vertecies.empty()) {
                if (game.ChunkUpdated) {
                    game.Frame += 1;

                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, vertecies.size() * sizeof(float), vertecies.data());

                    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &game.sizeInBytes);
                    GetProcessMemoryInfo(GetCurrentProcess(), &meminfo, sizeof(meminfo));
                    ramUsed = meminfo.WorkingSetSize;
                    
                    std::cout << "Sending: #" << game.Frame << " Info: verts=" << vertecies.size() << " chunks=" << World.size() << " Buffer size B: " 
                        << (vertecies.size() * sizeof(float))/1048576 << "MB/"<< game.sizeInBytes/1048576 << "MB" << " Render Dist:" << game.Render_Distance << " Ram Used:" << ramUsed / 1024 / 1024 << "MB"
                        << "\n";
                }
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertecies.size() / 5);
            }

            Fps.End();
        // Update Screen
            glfwSwapBuffers(window);
            glfwPollEvents();
    }
}

int main() {
    Game main;

    main.Init_Settings("MyCraft/Assets/Settings.txt");
    if (main.Init_Window()) return -1;
    
    main.Init_Shader();
    main.MainLoop();
    main.CleanUp();
    std::cout << "Safely Closed App";
    return 0;
}