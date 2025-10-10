#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <cmath>
#include <cinttypes>
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
#include "World/World.hpp"
#include "GUI/Gui.hpp"

glm::ivec3 Chunk_Size;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct Game_Settings {
    int Gui_Update_rate;
};

struct Game_Variables {
    int VramHandle;
    GLint sizeInBytes = 0;
    uint64_t Frame = 0;
    glm::ivec3 Last_Chunk;
    bool ChunkUpdated;
    float TickRate;
    float Tick_Timer = 0.0f;
    int V_Sync;
    int FOV;
    int FPS = 0;
    int Mesh_Updates;
    int World_Updates;
    int Updates;
    bool Gui_Init = false;

    // Terrain Generation:
    int Seed;
    float basefreq;
    float baseamp;
    int oct;
    float addfreq;
    float addamp;
    float biomefreq;
    float biomemult;
    float biomebase;
    int biomepower;
};

class Game {
private:
    FPS Fps;
    camera Camera;
    Game_Variables game;
    Game_Settings game_settings;
    GLuint ShaderProgram;
    size_t Mesh_Size;
    Movement movement;
    float DeltaTime;
    int width, height;
    colisions Colisions;
    Fun fun;
    Shader shader;
    Mesh mesh;
    size_t Alloc;
    size_t Capacity;
    Gui gui;

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

    Camera.RenderDistance = Settings.Get<int>("Render Distance", 0);
    game.VramHandle = Settings.Get<int>("Out Of Vram", 0);
    Chunk_Size.x = Settings.Get<int>("Chunk Width", 0);
    Chunk_Size.y = Settings.Get<int>("Chunk Height", 0);
    Chunk_Size.z = Settings.Get<int>("Chunk Depth", 0);
    game.V_Sync = Settings.Get<int>("V-Sync", 0);
    game.TickRate = 1.0f / Settings.Get<float>("Tick Rate", 0.0f);
    game.FOV = Settings.Get<int>("FOV", 0);
    Camera.Speed = Settings.Get<float>("Speed", 0.0f);
    Camera.SprintSpeed = Settings.Get<float>("Sprint Speed", 0.0f);
    game_settings.Gui_Update_rate = Settings.Get<int>("Gui Update Rate", 0.0);

    game.Seed = Settings.Get<int>("Seed", 0);
    game.basefreq = Settings.Get<float>("Base Frequency", 0.0f);
    game.baseamp = Settings.Get<float>("Base Amplitude", 0.0f);
    game.oct = Settings.Get<int>("Octave", 0);
    game.addfreq = Settings.Get<float>("Add Frequency", 0.0f);
    game.addamp = Settings.Get<float>("Add Amplitude", 0.0f);
    game.biomefreq = Settings.Get<float>("Biome Frequency", 0.0f);
    game.biomemult = Settings.Get<float>("Biome Multiplier", 0.0f);
    game.biomebase = Settings.Get<float>("Biome Add Amplitude", 0.0f);
    game.biomepower = Settings.Get<int>("Biome Power", 0);
    game.Mesh_Updates = Settings.Get<int>("Mesh Updates", 0);
    game.World_Updates = Settings.Get<int>("World Updates", 0);
}

void Game::CleanUp() {
    for (auto& [key, chunk] : World_Map::World) {
        chunk.RemoveData();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    game.Last_Chunk = glm::ivec3(999, 999, 999);
    
    return 0;
}

void Game::Init_Shader() {
    shader.Init_Shader(ShaderProgram);
}

void Tick_Update(camera &Camera, GLFWwindow* window, const float DeltaTime, Movement &movement, colisions &Colisions) {
    movement.Init(Camera, window, Chunk_Size, Colisions);
}

void DebugInfo(Game_Variables &game, const size_t Mesh_Size, const camera &Camera, const size_t Alloc, Fun &fun, const GLenum &err, const size_t Capacity) {
//----------------------
// Initialization
//----------------------
    const auto& World = World_Map::World;
    SIZE_T ramUsed;
    PROCESS_MEMORY_COUNTERS meminfo;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Debug");

    GetProcessMemoryInfo(GetCurrentProcess(), &meminfo, sizeof(meminfo));
    ramUsed = meminfo.WorkingSetSize;

//----------------------
// Performance
//----------------------
    ImGui::Text("FPS: %d", game.FPS);
    ImGui::Text("Triangles in Mesh: %s", fun.FormatNumber(Mesh_Size / 5).c_str());
    ImGui::Text("Chunks: %d", (int)World.size());
    ImGui::Text("Buffer Size: %s", fun.FormatSize(Mesh_Size).c_str());
    ImGui::Text("Tried to Alloc: %s", fun.FormatSize(Alloc).c_str());
    ImGui::Text("Capacity: %s", fun.FormatSize(Capacity).c_str());
    ImGui::Text("Render Distance: %d", Camera.RenderDistance);
    ImGui::Text("Ram Used: %dMB", ramUsed / 1024 / 1024);
    ImGui::Text("World Usage: %zuMB", fun.calculateWorldMemory(World, Chunk_Size) / (1024 * 1024));
    ImGui::Text("OpenGL error: 0x%X", err);

//----------------------
// Player
//----------------------
    ImGui::Text("");
    ImGui::Text("Camera:");
    std::string Mode;
    if (!Camera.Mode) Mode = "Survival"; else Mode = "Spectator";
    ImGui::Text("Mode: %s", Mode.c_str());
    ImGui::Text("Position: X:%.1f Y:%.1f Z:%.1f", Camera.Position.x, Camera.Position.y, Camera.Position.z);
    ImGui::Text("Chunk: X:%d Z:%d", Camera.Chunk.x, Camera.Chunk.z);
    ImGui::Text("Sub Chunk Y: %d", static_cast<int>(std::floor(Camera.Position.y / 16.0f)));

    ImGui::End();
    ImGui::Render();
}

void Game::MainLoop() {
    ChunkGeneration GenerateChunk(game.Seed, game.basefreq, game.baseamp, game.oct, game.addfreq, game.addamp, game.biomefreq, game.biomemult, game.biomebase, game.biomepower);
    glfwGetWindowSize(window, &width, &height);
    Fps.Init();
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

            glUseProgram(ShaderProgram);
        // -------------------------------------------------------------------------------
        // Main Engine
        // -------------------------------------------------------------------------------

            DeltaTime = Fps.Start();
            game.Tick_Timer += DeltaTime;
            game.Frame += 1;

            //-------------------------
            // Tick Update
            //-------------------------
            while (game.Tick_Timer >= game.TickRate) {
                game.Tick_Timer -= game.TickRate;
                if (!game.ChunkUpdated) {
                    Tick_Update(Camera, window, DeltaTime, movement, Colisions);
                }

            //-------------------------
            // Mesh Generation
                game.Updates = 0;
                for (auto& [key, chunk] : World_Map::World) {
                    if (chunk.DirtyFlag && chunk.Gen_Mesh) {
                        if (game.Updates <= game.Mesh_Updates) {
                            chunk.Allocate();
                            mesh.GenerateMesh(chunk, chunk.Mesh, key.first, key.second, Chunk_Size, Camera.RenderDistance);
                            chunk.SendData();
                            chunk.Gen_Mesh = false;
                            game.Updates += 1;
                        } else {
                            break;
                        }
                    }
                }
            }

        //-------------------------
        // Clearing Screen
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       

        //-------------------------
        // Uniforms
        //-------------------------
            const float aspectRatio = (float)width / (float)height;
            const float FOV = fun.ConvertHorizontalFovToVertical(game.FOV, aspectRatio);

            const glm::mat4 model = glm::mat4(1.0f);
            const glm::mat4 view = movement.GetViewMatrix(Camera);
            const glm::mat4 proj = glm::perspective(glm::radians(FOV), aspectRatio, 0.1f, 2000.0f);

            shader.Set_Vec3(ShaderProgram, "ViewPos", Camera.Position);
            shader.Set_Mat4(ShaderProgram, "Model", model);
            shader.Set_Mat4(ShaderProgram, "View", view);
            shader.Set_Mat4(ShaderProgram, "Proj", proj);

            Camera.Chunk.x = static_cast<int>(std::floor(Camera.Position.x / Chunk_Size.x));
            Camera.Chunk.y = 0;
            Camera.Chunk.z = static_cast<int>(std::floor(Camera.Position.z / Chunk_Size.z));
        
        //-------------------------
        // Chunk Update
            game.ChunkUpdated = false;
            if (Camera.Chunk != game.Last_Chunk) {
                game.ChunkUpdated = true;
                game.Last_Chunk = Camera.Chunk;
            }

        //-------------------------
        // World/Mesh Generation
        //-------------------------
            if (game.ChunkUpdated) {
                if (game.World_Updates == 0) {
                    GenerateChunk.GenerateChunks(Camera, Chunk_Size);
                }
                
                GenerateChunk.RemoveChunks(Camera);
                
                if (game.Mesh_Updates == 0) {
                    for (auto& [key, chunk] : World_Map::World) {
                        if (chunk.DirtyFlag) {
                            chunk.Allocate();
                            mesh.GenerateMesh(chunk, chunk.Mesh, key.first, key.second, Chunk_Size, Camera.RenderDistance);
                            chunk.SendData();
                        }
                    }
                }
            }
        
        //-------------------------
        // Drawing Mesh to Screen
        //-------------------------
            Alloc = 0;
            Capacity = 0;
            Mesh_Size = 0;
            for (const auto& [key, chunk] : World_Map::World) {
                //if (!(Mesh_Size * sizeof(float)/1048576 > game.VRamAlloc)) {
                    glBindVertexArray(chunk.vao);
                    glDrawArrays(GL_TRIANGLES, 0, chunk.indexCount);
                    glBindVertexArray(0);
                //}
                Alloc += chunk.Alloc;
                Mesh_Size += chunk.Mesh.size();
                Capacity += chunk.Mesh.capacity();
            }
        //-------------------------
        // GUI - My Own GUI Engine
        //-------------------------
        GLenum err = glGetError();
        if (!game.Gui_Init) {
            // Initialize Gui
            //gui.addWidget<Label>(10,10,"Hello");
            //gui.addWidget<Button>(10,10,50,50,"Button");
            game.Gui_Init = true;
            DebugInfo(game, Mesh_Size, Camera, Alloc, fun, err, Capacity);
        }

        if (game.Frame % game_settings.Gui_Update_rate == 0) {
            // Update Gui
            //gui.update(0,0,false);
            //gui.render();
            DebugInfo(game, Mesh_Size, Camera, Alloc, fun, err, Capacity);
        }
        // Render ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //-------------------------
        // Out Of VRam Error
        //-------------------------
            if (err == GL_OUT_OF_MEMORY) {
                if (game.VramHandle == 1) {
                    std::cerr << "Out of VRAM! Changed RenderDistance by -1" << "\n";
                    if (Camera.RenderDistance > 1) {
                        Camera.RenderDistance -= 1;
                    }
                } else {
                    break;
                }
            }


            DebugInfo(game, Mesh_Size, Camera, Alloc, fun, err, Capacity);
            game.FPS = Fps.End();
        // Update Screen
            glfwSwapBuffers(window);
            glfwPollEvents();
    }
}

int main() {
    Game main;

    main.Init_Settings("MyCraft/Assets/Settings.myc");
    if (main.Init_Window()) return -1;
    
    main.Init_Shader();
    main.MainLoop();
    main.CleanUp();
    std::cout << "Safely Closed App";
    return 0;
}