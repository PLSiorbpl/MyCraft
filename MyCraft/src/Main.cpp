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
#if defined(_WIN32) // Windows
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__) // Linux
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/sysinfo.h>
#endif
#include <fstream>
#include <sstream>
#include <filesystem>
#include "FastNoiseLite.h"
#define STB_IMAGE_IMPLEMENTATION
// My Files
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
#include "Render/Frustum.hpp"
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
    int ramHandle;
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
    int Lazy_Mesh_Updates;
    int World_Updates;
    int Updates;
    bool Gui_Init = false;
    uint64_t Max_Ram;

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
    int Triangles;
    Gui gui;
    Frustum frustum;
    #if defined(_WIN32) // Windows
        PROCESS_MEMORY_COUNTERS meminfo;
        SIZE_T ramUsed;
    #elif defined(__linux__) // Linux
        size_t ramUsed;
    #endif

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
    game.Max_Ram = Settings.Get<int>("RAM", 0);
    game.ramHandle = Settings.Get<int>("Out Of ram", 0);
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
    game.Lazy_Mesh_Updates = Settings.Get<int>("Lazy Mesh Updates", 0);
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

    window = glfwCreateWindow(width, height, "MyCraft", monitor, nullptr);
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
    glfwSetWindowPos(window, 0, 0);
    glfwSetWindowSize(window, mode->width, mode->height);

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
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

void DebugInfo(Game_Variables &game, const size_t Mesh_Size, const camera &Camera, const size_t Alloc, Fun &fun, const GLenum &err, const size_t Capacity, size_t ramUsed, int Triangles) {
//----------------------
// Initialization
//----------------------
    const auto& World = World_Map::World;
    const float ramUsedRatio = float(ramUsed) / float(game.Max_Ram * 1024 * 1024);
    const float TrianglesDrawnRatio = float(Triangles/3) / float(Mesh_Size/3);
    const ImVec4 RamBarColor = (ramUsedRatio > 0.8f) ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) :
                    (ramUsedRatio > 0.5f) ? ImVec4(1.0f, 0.7f, 0.2f, 1.0f) :
                                             ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
    const ImVec4 TriBarColor = (TrianglesDrawnRatio > 0.8f) ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) :
                    (TrianglesDrawnRatio > 0.5f) ? ImVec4(1.0f, 0.7f, 0.2f, 1.0f) :
                                             ImVec4(0.2f, 1.0f, 0.2f, 1.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Debug");

//----------------------
// Performance
//----------------------
    ImGui::Text("FPS: %d", game.FPS);

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, TriBarColor);
    std::string overlay = fun.FormatNumber(Triangles/3) + "/" + fun.FormatNumber(Mesh_Size/3);
    ImGui::Text("Triangles: ");
    ImGui::SameLine();
    ImGui::ProgressBar(TrianglesDrawnRatio, ImVec2(0.0f, 0.0f), overlay.c_str());
    ImGui::PopStyleColor();

    ImGui::Text("Chunks: %s", fun.FormatNumber(World.size()).c_str());
    ImGui::Text("VRam Usage: %s", fun.FormatSize(Mesh_Size).c_str());
    ImGui::Text("Mesh Allocated: %s", fun.FormatSize(Alloc).c_str());
    ImGui::Text("Mesh Capacity: %s", fun.FormatSize(Capacity).c_str());
    ImGui::Text("Render Distance: %d", Camera.RenderDistance);

    overlay = fun.FormatSize(ramUsed) + "/" + fun.FormatSize(game.Max_Ram * 1024 * 1024);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, RamBarColor);
    ImGui::Text("Ram:");
    ImGui::SameLine();
    ImGui::ProgressBar(ramUsedRatio, ImVec2(0.0f, 0.0f), overlay.c_str());
    ImGui::PopStyleColor();

    ImGui::Text("World Usage: %s", fun.FormatSize(fun.calculateWorldMemory(World, Chunk_Size)).c_str());
    ImGui::Text("OpenGL error: 0x%X", err);
    ImGui::Text("Mesh Updates: %i/%i", game.Updates, game.Mesh_Updates);

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
            shader.Set_Int(ShaderProgram, "RenderDist", Camera.RenderDistance);
    
            const Frustum::Frust Frust = frustum.ExtractFrustum(proj*view);
    
            Camera.Chunk.x = static_cast<int>(std::floor(Camera.Position.x / Chunk_Size.x));
            Camera.Chunk.y = 0;
            Camera.Chunk.z = static_cast<int>(std::floor(Camera.Position.z / Chunk_Size.z));
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
                if (!chunk.DirtyFlag || !chunk.Gen_Mesh)
                    continue;

                if (game.Updates >= game.Mesh_Updates)
                    break;
            
                const glm::vec3 chunkMin = glm::vec3(key.first * Chunk_Size.x, 0, key.second * Chunk_Size.z);
                const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk_Size);
            
                const bool visible = frustum.IsAABBVisible(Frust, chunkMin, chunkMax);

                // Normal Updates
                if (visible && game.Updates < game.Mesh_Updates) {
                    chunk.Allocate();
                    mesh.GenerateMesh(chunk, chunk.Mesh, key.first, key.second, Chunk_Size, Camera.RenderDistance);
                    chunk.SendData();
                    chunk.Gen_Mesh = false;
                    chunk.Ready_Render = true;
                    game.Updates++;
                    continue;
                }
            
                // Lazy Updates
                if (!visible && game.Updates < game.Lazy_Mesh_Updates) {
                    chunk.Allocate();
                    mesh.GenerateMesh(chunk, chunk.Mesh, key.first, key.second, Chunk_Size, Camera.RenderDistance);
                    chunk.SendData();
                    chunk.Gen_Mesh = false;
                    chunk.Ready_Render = true;
                    game.Updates++;
                }
            }
            }

        //-------------------------
        // Clearing Screen
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       

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
                        if (chunk.DirtyFlag && chunk.Gen_Mesh) {
                            chunk.Allocate();
                            mesh.GenerateMesh(chunk, chunk.Mesh, key.first, key.second, Chunk_Size, Camera.RenderDistance);
                            chunk.SendData();
                            chunk.Gen_Mesh = false;
                            chunk.Ready_Render = true;
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
            Triangles = 0;
            for (const auto& [key, chunk] : World_Map::World) {
                //if (!(Mesh_Size * sizeof(float)/1048576 > game.VRamAlloc)) {
                if (chunk.Ready_Render) {
                    const glm::vec3 chunkMin = glm::vec3(key.first * Chunk_Size.x, 0, key.second * Chunk_Size.z);
                    const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk_Size);

                    if (frustum.IsAABBVisible(Frust, chunkMin, chunkMax)) {
                        glBindVertexArray(chunk.vao);
                        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        glDrawArrays(GL_TRIANGLES, 0, chunk.indexCount);
                        glBindVertexArray(0);
                        Triangles += chunk.Mesh.size();
                    }
                }
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
            DebugInfo(game, Mesh_Size, Camera, Alloc, fun, err, Capacity, ramUsed, Triangles);
        }

        if (game.Frame % game_settings.Gui_Update_rate == 0) {
            // Update Gui
            //gui.update(0,0,false);
            //gui.render();
            DebugInfo(game, Mesh_Size, Camera, Alloc, fun, err, Capacity, ramUsed, Triangles);
        }
        // Render ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //-------------------------
        // Out Of VRam Error
        //-------------------------
        #if defined(_WIN32)
            GetProcessMemoryInfo(GetCurrentProcess(), &meminfo, sizeof(meminfo));
            ramUsed = meminfo.WorkingSetSize;
        #elif defined(__linux__)
            std::ifstream file("/proc/self/statm");
            size_t size;
            file >> size;
            long page_size_kb = sysconf(_SC_PAGE_SIZE);
            ramUsed = size * page_size_kb;
        #endif
            if (err == GL_OUT_OF_MEMORY || ramUsed >= game.Max_Ram*1024*1024) {
                if (game.ramHandle == 1) {
                    std::cerr << "Out of VRAM! Changed RenderDistance by -1" << "\n";
                    if (Camera.RenderDistance > 1) {
                        Camera.RenderDistance -= 1;
                        GenerateChunk.RemoveChunks(Camera);
                    }
                } else {
                    break;
                }
            }
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