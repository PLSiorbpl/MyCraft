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
#include "FastNoiseLite.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Utils/FPS.hpp"
#include "Render/Camera.hpp"
#include "Utils/Settings.hpp"
#include "World/Chunk.hpp"
#include "Player/Movement.hpp"
#include "Shader_Utils/Shader.hpp"
#include "Utils/Function.hpp"

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
    int Seed;
    int FOV;
};

struct Entity {
    glm::vec3 Pos;
    glm::ivec3 Chunk;
    float Speed;
};

void Get_World_Chunk(int Chunk_x, int Chunk_z, std::map<std::pair<int, int>, Chunk>& World, int Seed) {
    Chunk Chunk;
    Chunk.width = CHUNK_WIDTH;
    Chunk.height = CHUNK_HEIGHT;
    Chunk.depth = CHUNK_DEPTH;
    Chunk.chunkX = Chunk_x;
    Chunk.chunkZ = Chunk_z;
    Chunk.blocks.resize(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH);

    FastNoiseLite terrainNoise;
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    terrainNoise.SetSeed(Seed);

    FastNoiseLite biomeNoise;
    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    biomeNoise.SetSeed(Seed + 1337);

    const float baseFreq = 0.5f;
    const float baseAmp = 0.2f;
    const int octaves = 4;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            float worldX = Chunk_x * CHUNK_WIDTH + x;
            float worldZ = Chunk_z * CHUNK_DEPTH + z;

            float biomeFactor = biomeNoise.GetNoise(worldX * 0.003f, worldZ * 0.003f);
            biomeFactor = biomeFactor * 0.5f + 0.5f;

            float total = 0.0f;
            float freq = baseFreq;
            float amp = baseAmp;

            for (int i = 0; i < octaves; ++i) {
                total += terrainNoise.GetNoise(worldX * freq, worldZ * freq) * amp;
                freq *= 2.0f;
                amp *= 0.2f;
            }

            float height_f = total * 0.5f + 0.5f;

            height_f = height_f * (0.8f + biomeFactor * 1.2f);

            int height = static_cast<int>(height_f * CHUNK_HEIGHT);

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


void Generate_Chunks(const Entity& Player, int Render_Dist, std::map<std::pair<int, int>, Chunk>& World, camera &Camera, int Seed) {
    glm::ivec3 Start_Chunk = Player.Chunk;

    for (int dx = -Render_Dist; dx <= Render_Dist; ++dx) {
        for (int dz = -Render_Dist; dz <= Render_Dist; ++dz) {
            int chunkX = Start_Chunk.x + dx;
            int chunkZ = Start_Chunk.z + dz;

            std::pair<int, int> key = {chunkX, chunkZ};

            if (World.find(key) == World.end()) {
                Get_World_Chunk(chunkX, chunkZ, World, Seed);
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

void Input_Handler(camera &Camera, GLFWwindow* window, float deltaTime, std::map<std::pair<int, int>, Chunk>& World, Movement &movement, colisions &Colisions) {
    movement.Init(Camera);
    movement.Input(window, Camera);
    movement.TestColisions(Camera, World, glm::vec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH), Colisions);
    movement.Damp(Camera);
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
    Movement movement;
    float DeltaTime;
    int width, height;
    colisions Colisions;
    Fun fun;

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
    game.Seed = Settings.GetInt("Seed");
    game.FOV = Settings.GetInt("FOV");
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
    Shader shader;

    shader.Init_Shader(game.VRamAlloc, VAO, VBO, ShaderProgram);
}

void Tick_Update(camera &Camera, GLFWwindow* window, float &DeltaTime, std::map<std::pair<int, int>, Chunk> &World, Movement &movement, colisions &Colisions) {
    Input_Handler(Camera, window, DeltaTime, World, movement, Colisions);
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
                    Tick_Update(Camera, window, DeltaTime, World, movement, Colisions);
                }
            }

        // Clearing
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MVP
            float aspectRatio = (float)width / (float)height;
            float FOV = fun.ConvertHorizontalFovToVertical(game.FOV, aspectRatio);

            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = GetViewMatrix(Camera);
            glm::mat4 proj = glm::perspective(glm::radians(FOV), aspectRatio, 0.1f, 600.0f);
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
                Generate_Chunks(Player, game.Render_Distance, World, Camera, game.Seed);
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