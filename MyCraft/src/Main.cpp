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
#include "World/Terrain.hpp"
#include "World/Generation.hpp"
#include "Render/Mesh.hpp"

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

class Game {
private:
    SIZE_T ramUsed;
    PROCESS_MEMORY_COUNTERS meminfo;
    FPS Fps;
    camera Camera;
    Game_Variables game;
    GLuint VAO, VBO, ShaderProgram;
    std::vector<float> vertecies;
    std::map<std::pair<int, int>, Chunk> World;
    Movement movement;
    float DeltaTime;
    int width, height;
    colisions Colisions;
    Fun fun;
    Shader shader;
    Mesh mesh;

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

    Camera.RenderDistance = Settings.GetInt("Render Distance");
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
    glfwSetCursorPosCallback(window, movement.mouse_callback);

    game.Last_Chunk = glm::ivec3(999, 999, 999);
    
    return 0;
}

void Game::Init_Shader() {
    shader.Init_Shader(game.VRamAlloc, VAO, VBO, ShaderProgram);
    // Soon more shaders
}

void Tick_Update(camera &Camera, GLFWwindow* window, float &DeltaTime, std::map<std::pair<int, int>, Chunk> &World, Movement &movement, colisions &Colisions) {
    movement.Init(Camera, window, World, glm::ivec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH), Colisions);
}

void Game::MainLoop() {
    ChunkGeneration GenerateChunk(game.Seed);
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
            glm::mat4 view = movement.GetViewMatrix(Camera);
            glm::mat4 proj = glm::perspective(glm::radians(FOV), aspectRatio, 0.1f, 600.0f);
            glm::mat4 MVP = proj * view * model;

            GLuint mvpLoc = glGetUniformLocation(ShaderProgram, "MVP");
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &MVP[0][0]);

            Camera.Chunk.x = static_cast<int>(std::floor(Camera.Position.x / CHUNK_WIDTH));
            Camera.Chunk.y = 0;
            Camera.Chunk.z = static_cast<int>(std::floor(Camera.Position.z / CHUNK_DEPTH));

        // Update Logic
            game.ChunkUpdated = false;
            if (Camera.Chunk != game.Last_Chunk) {
                game.ChunkUpdated = true;
                game.Last_Chunk = Camera.Chunk;
            }

        // Generating Chunks
            if (game.ChunkUpdated) {
                // Generate Chunks & Remove them if needed
                GenerateChunk.GenerateChunks(Camera, World, glm::ivec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH));
                GenerateChunk.RemoveChunks(Camera, World);

                // Generating Mesh
                vertecies.clear();
                for (auto& [key, chunk] : World) {
                    mesh.GenerateMesh(chunk, vertecies, key.first, key.second, glm::ivec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH), Camera.RenderDistance);
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