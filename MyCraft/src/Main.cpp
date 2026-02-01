#define WIN32_LEAN_AND_MEAN
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
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
#define STB_IMAGE_IMPLEMENTATION

// My Files
#include "GUI/Gui.hpp"

#include "Network/Network.hpp"

#include "Player/Movement.hpp"

#include "Render/Mesh.hpp"
#include "Render/Frustum.hpp"
#include "Render/SelectionBox.hpp"
#include "Render/Camera.hpp"

#include "Shader_Utils/Shader.hpp"

#include "Utils/FPS.hpp"
#include "Utils/Settings.hpp"
#include "Utils/Function.hpp"
#include "Utils/Timer.hpp"
#include "Utils/Globals.hpp"

#include "World/Chunk.hpp"
#include "World/Generation.hpp"
#include "World/World.hpp"

auto Chunk_Size = glm::ivec3(16);

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    const auto* ctx = static_cast<window_context*>(
        glfwGetWindowUserPointer(window)
    );
    ctx->game_settings->width = width;
    ctx->game_settings->height = height;
    glViewport(0, 0, width, height);
}

class Game {
private:
    Timer time;
    Timer FrameTime;
    FPS Fps;
    Movement movement = {};
    Fun fun;
    Shader shader;
    Mesh mesh;
    Gui gui = {};
    Frustum frustum;
    Selection selection = {};
public:
    static window_context ctx;
    Settings_Loader Settings;

    static bool Init_Window();

    static void Init_Shader();
    void MainLoop();
    static void CleanUp();
    void Init_Settings(const std::string& Path);
};

window_context Game::ctx = {&Camera, &game_settings};

void Game::Init_Settings(const std::string& Path) {
    Settings.Load_Settings(Path);

    // General Options:
    Camera.RenderDistance = Settings.Get<int>("Render Distance", 2);
    game_settings.Generation_Threads = Settings.Get<unsigned int>("Generation Threads", 2);

    game.Max_Ram = Settings.Get<int>("RAM", 2048);
    game.V_Sync = Settings.Get<int>("V-Sync", 0);

    game.Mesh_Updates = Settings.Get<int>("Mesh Updates", 0);
    game.Lazy_Mesh_Updates = Settings.Get<int>("Lazy Mesh Updates", 0);
    game.ramHandle = Settings.Get<int>("Out Of ram", 1);

    // World Generation:
    Chunk_Size.x = Settings.Get<int>("Chunk Width", 16);
    Chunk_Size.y = Settings.Get<int>("Chunk Height", 256);
    Chunk_Size.z = Settings.Get<int>("Chunk Depth", 16);
    game_settings.World_Generation_Type = Settings.Get<int>("Generation Type", 0);

    terrain_settings.Seed = Settings.Get<int>("Seed", 0);
    terrain_settings.basefreq = Settings.Get<float>("Base Frequency", 0.0f);
    terrain_settings.baseamp = Settings.Get<float>("Base Amplitude", 0.0f);
    terrain_settings.oct = Settings.Get<int>("Octave", 0);
    terrain_settings.addfreq = Settings.Get<float>("Add Frequency", 0.0f);
    terrain_settings.addamp = Settings.Get<float>("Add Amplitude", 0.0f);

    terrain_settings.biomefreq = Settings.Get<float>("Biome Frequency", 0.0f);
    terrain_settings.biomemult = Settings.Get<float>("Biome Multiplier", 0.0f);
    terrain_settings.biomebase = Settings.Get<float>("Biome Add Amplitude", 0.0f);
    terrain_settings.biomepower = Settings.Get<float>("Biome Power", 0.0f);

    // Game:
    game.TickRate = 1.0f / Settings.Get<float>("Tick Rate", 60.0f);

    // Player:
    game.FOV = Settings.Get<float>("FOV", 80);
    Camera.Speed = Settings.Get<float>("Speed", 0.0f);
    Camera.SprintSpeed = Settings.Get<float>("Sprint Speed", 0.0f);
}

void Game::CleanUp() {
    glFinish();
    for (auto& [key, chunk] : World_Map::World) {
        chunk.RemoveData();
    }
    net.client.Stop_Client();
    net.server.Stop_Server();
    glDeleteProgram(SH.Solid_Shader_Blocks.Shader);
    glDeleteProgram(SH.General_Gui_Shader.Shader);
    glDeleteProgram(SH.SelectionBox_Shader.Shader);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Game::Init_Window() {
     if (!glfwInit()) {
        std::cerr << "Cant Initialize GLFW (skill issue)!\n";
        return true;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor); // Window or Full Screen

    game_settings.width  = mode->width;
    game_settings.height = mode->height;

    window = glfwCreateWindow(game_settings.width, game_settings.height, "MyCraft", monitor, nullptr);

    if (!window) {
        std::cerr << "Your GPU doesnt Support OpenGl 4.6. Trying 3.3\n";

        // Fallback to 3.3 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(game_settings.width, game_settings.height, "MyCraft", monitor, nullptr);

        if (!window) {
            std::cerr << "Your GPU doesnt Support OpenGl 3.3. Try updating Drivers\n";
            glfwTerminate();
            return false;
        }
    }
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowPos(window, 0, 0);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(game.V_Sync); // V-sync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Glad loader error\n";
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetMouseButtonCallback(window, InputManager::Mouse_Key_Callback);
    glfwSetCharCallback(window, InputManager::Char_Callback);
    glfwSetKeyCallback(window, InputManager::Key_Callback);
    glfwSetScrollCallback(window, InputManager::Scroll_Callback);
    glfwSetCursorPosCallback(window, InputManager::Mouse_Callback);

    glfwSetWindowUserPointer(window, &ctx);

    game.Last_Chunk = glm::ivec3(999, 999, 999);

    glGetIntegerv(GL_MAJOR_VERSION, &PerfS.major);
    glGetIntegerv(GL_MINOR_VERSION, &PerfS.minor);

    PerfS.isModernGL = (PerfS.major >= 4);
    
    return false;
}

void Game::Init_Shader() {
    Shader::Init_Shader();
}

void Tick_Update(GLFWwindow* window, Movement &movement, Selection &Sel) {
    movement.Init(window, Chunk_Size, Sel);
}

void Game::MainLoop() {
    ChunkGeneration GenerateChunk;
    glfwGetWindowSize(window, &game_settings.width, &game_settings.height);
    selection.Init(SH.SelectionBox_Shader.Shader);
    Fps.Init();
    GenerateChunk.Start(game_settings.Generation_Threads, Chunk_Size);
    while (!glfwWindowShouldClose(window)) {

            game.DeltaTime = Fps.Start();
            FrameTime.Reset();
            if (game_settings.width == 0 || game_settings.height == 0) {
                glfwPollEvents();
                continue;
            }
            glfwPollEvents();
            
        // -------------------------------------------------------------------------------
        // Main Engine
        // -------------------------------------------------------------------------------
            //-------------------------
            // Uniforms
            //-------------------------
            const float aspectRatio = static_cast<float>(game_settings.width) / static_cast<float>(std::max(game_settings.height, 1));
            const float FOV = Fun::ConvertHorizontalFovToVertical(game.FOV, aspectRatio);

            static constexpr auto model = glm::mat4(1.0f);
            const glm::mat4 view = Movement::GetViewMatrix();
            const glm::mat4 proj = glm::perspective(glm::radians(FOV), aspectRatio, 0.1f, 2000.0f);

            glUseProgram(SH.Solid_Shader_Blocks.Shader);
            Shader::Set_Int(SH.Solid_Shader_Blocks.Shader, "BaseTexture", 0);
            Shader::Set_Vec3(SH.Solid_Shader_Blocks.Shader, "ViewPos", Camera.Position);
            Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Model", model);
            Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "View", view);
            Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Proj", proj);
            Shader::Set_Int(SH.Solid_Shader_Blocks.Shader, "RenderDist", Camera.RenderDistance);
    
            const Frustum::Frust Frust = Frustum::ExtractFrustum(proj*view);
    
            Camera.Chunk.x = static_cast<int>(std::floor(Camera.Position.x / static_cast<float>(Chunk_Size.x)));
            Camera.Chunk.y = 0;
            Camera.Chunk.z = static_cast<int>(std::floor(Camera.Position.z / static_cast<float>(Chunk_Size.z)));
            game.Tick_Timer += game.DeltaTime;
            game.Frame += 1;

            //-------------------------
            // Chunk Update
            game.ChunkUpdated = false;
            if (Camera.Chunk != game.Last_Chunk) {
                game.ChunkUpdated = true;
                game.Last_Chunk = Camera.Chunk;
            }

            //-------------------------
            // Tick Update
            //-------------------------
            time.Reset();
            while (game.Tick_Timer >= game.TickRate) {
                game.Tick_Timer -= game.TickRate;
                if (!game.ChunkUpdated) {
                    Tick_Update(window, movement, selection);
                }
            }
            PerfS.tick = time.ElapsedMs();
            //-------------------------
            // Mesh Generation
            if (!World_Map::Render_List.empty()) {
                for (size_t i = World_Map::Render_List.size(); i-- > 0;) {
                    auto& info = World_Map::Render_List[i];
                    if (info.Delete == 5) {
                    
                        auto& chunk = World_Map::World.find({info.chunkX, info.chunkZ})->second;
                        chunk.InRender = false;
                    
                        glDeleteBuffers(1, &info.vbo);
                        glDeleteVertexArrays(1, &info.vao);
                    
                        if (i != World_Map::Render_List.size() - 1)
                            std::swap(World_Map::Render_List[i], World_Map::Render_List.back());
                    
                        World_Map::Render_List.pop_back();
                    }
                }
            }

            game.Updates = 0;
            time.Reset();
            for (Chunk* chunk : World_Map::Mesh_Queue) {
                if (!chunk->DirtyFlag || !chunk->Gen_Mesh || chunk->InRender)
                    continue;
                if (game.Updates >= game.Mesh_Updates)
                    break;

                const auto chunkMin = glm::vec3(chunk->chunkX * Chunk_Size.x, 0, chunk->chunkZ * Chunk_Size.z);
                const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk_Size);
            
                const bool visible = Frustum::IsAABBVisible(Frust, chunkMin, chunkMax);
                // Normal Updates
                if (visible && game.Updates < game.Mesh_Updates) {
                    const int chunkX = chunk->chunkX;
                    const int chunkZ = chunk->chunkZ;
                    const std::array<std::pair<int,int>, 4> neighbors = {
                        std::make_pair(chunkX+1, chunkZ),
                        std::make_pair(chunkX-1, chunkZ),
                        std::make_pair(chunkX, chunkZ+1),
                        std::make_pair(chunkX, chunkZ-1)
                    };
                    bool skip = false;
                    for (auto& n : neighbors) {
                        if (World_Map::World.find(n) == World_Map::World.end()) {
                            skip = true;
                            break;
                        }
                    }
                    if (skip) continue;

                    chunk->Mesh.clear();
                    Mesh::GenerateMesh(*chunk, chunk->Mesh, chunk->chunkX, chunk->chunkZ, Chunk_Size, Camera.RenderDistance);
                    chunk->SendData();
                    World_Map::Render_List.push_back({
                        chunk->chunkX,
                        chunk->chunkZ,
                        chunk->vao,
                        chunk->vbo,
                        chunk->indexCount,
                        chunk->Mesh.size()*sizeof(Chunk::Vertex),
                        chunk->Mesh.capacity()*sizeof(Chunk::Vertex),
                        chunk->Mesh.size()/3,
                        0
                    });
                    chunk->vao = 0;
                    chunk->vbo = 0;
                    chunk->Mesh.clear();
                    chunk->Mesh.shrink_to_fit();
                    chunk->Gen_Mesh = false;
                    chunk->Ready_Render = true;
                    chunk->InRender = true;
                    game.Updates++;
                    continue;
                }
            
                // Lazy Updates
                if (!visible && game.Updates < game.Lazy_Mesh_Updates) {
                    const int chunkX = chunk->chunkX;
                    const int chunkZ = chunk->chunkZ;
                    const std::array<std::pair<int,int>, 4> neighbors = {
                        std::make_pair(chunkX+1, chunkZ),
                        std::make_pair(chunkX-1, chunkZ),
                        std::make_pair(chunkX, chunkZ+1),
                        std::make_pair(chunkX, chunkZ-1)
                    };
                    bool skip = false;
                    for (auto& n : neighbors) {
                        if (World_Map::World.find(n) == World_Map::World.end()) {
                            skip = true;
                            break;
                        }
                    }
                    if (skip) continue;

                    chunk->Mesh.clear();
                    Mesh::GenerateMesh(*chunk, chunk->Mesh, chunk->chunkX, chunk->chunkZ, Chunk_Size, Camera.RenderDistance);
                    chunk->SendData();
                    World_Map::Render_List.push_back({
                        chunk->chunkX,
                        chunk->chunkZ,
                        chunk->vao,
                        chunk->vbo,
                        chunk->indexCount,
                        chunk->Mesh.size()*sizeof(Chunk::Vertex),
                        chunk->Mesh.capacity()*sizeof(Chunk::Vertex),
                        chunk->Mesh.size()/3,
                        0
                    });
                    chunk->vao = 0;
                    chunk->vbo = 0;
                    chunk->Mesh.clear();
                    chunk->Mesh.shrink_to_fit();
                    chunk->Gen_Mesh = false;
                    chunk->Ready_Render = true;
                    chunk->InRender = true;
                    game.Updates++;
                }
            }
            // Delete already done chunks
            if (!World_Map::Mesh_Queue.empty()) {
                for (size_t i = World_Map::Mesh_Queue.size(); i-- > 0;) {
                    Chunk* chunk = World_Map::Mesh_Queue[i];

                    if (!chunk->Ready_Render)
                        continue;

                    if (i != World_Map::Mesh_Queue.size() - 1)
                        std::swap(World_Map::Mesh_Queue[i], World_Map::Mesh_Queue.back());
                    World_Map::Mesh_Queue.pop_back();
                }
            }
            PerfS.mesh = time.ElapsedMs();

        //-------------------------
        // Clearing Screen
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //-------------------------
        // World Generation
        //-------------------------
        time.Reset();
        {
                std::lock_guard lock(GenerateChunk.ResultMutex);
                for (auto& r : GenerateChunk.ReadyChunks) {
                    World_Map::World[{r.chunkX, r.chunkZ}] = r;
                    World_Map::Mesh_Queue.push_back(&World_Map::World[{r.chunkX, r.chunkZ}]);
                }
                GenerateChunk.ReadyChunks.clear();
        }

            if (game.ChunkUpdated) {
                if (game.World_Updates == 0) {
                    GenerateChunk.LookForChunks();
                    PerfS.chunk = time.ElapsedMs();
                }
                time.Reset();
                ChunkGeneration::RemoveChunks();
                PerfS.remove = time.ElapsedMs();
            }
        
        //-------------------------
        // Drawing Mesh to Screen
        //-------------------------
            time.Reset();
            PerfS.Capacity = 0; PerfS.Mesh_Size = 0; PerfS.Triangles = 0; PerfS.Total_Triangles = 0;
        if (game_settings.width != 0 && game_settings.height != 0) {
            for (auto& info : World_Map::Render_List) {
                if (info.Delete > 0) {
                    info.Delete++;
                    //continue;
                }

                const auto chunkMin = glm::vec3(info.chunkX * Chunk_Size.x, 0, info.chunkZ * Chunk_Size.z);
                const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk_Size);

                if (Frustum::IsAABBVisible(Frust, chunkMin, chunkMax)) {
                    glBindVertexArray(info.vao);
                    glDrawArrays(GL_TRIANGLES, 0, info.indexCount);
                    PerfS.Triangles += info.Triangles;
                }
                PerfS.Mesh_Size += info.Mesh_Size;
                PerfS.Capacity += info.Capacity;
                PerfS.Total_Triangles += info.Triangles;
            }
        if (Camera.Draw_Selection) {
            glUseProgram(SH.SelectionBox_Shader.Shader);
            glBindVertexArray(selection.vao);
            glBindBuffer(GL_ARRAY_BUFFER, selection.vbo);
            const auto size = static_cast<GLsizeiptr>(selection.boxLinesCopy.size() * sizeof(float));
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, selection.boxLinesCopy.data());
            glm::mat4 MVP = proj * view * model;
            Shader::Set_Mat4(SH.SelectionBox_Shader.Shader, "MVP", MVP);
            glLineWidth(1.0f);
            glDrawArrays(GL_LINES, 0, 24);
        }
        }
        PerfS.render = time.ElapsedMs();
        
        //-------------------------
        // GUI - My Own GUI Engine
        //-------------------------
            time.Reset();
            gui.backend.ResetFrame();
            gui.Generate();
            gui.backend.SendMesh();
            gui.backend.RenderFrame();
            PerfS.gui = time.ElapsedMs();

        //-------------------------
        // Out Of VRam Error
        //-------------------------
        #if defined(_WIN32)
            GetProcessMemoryInfo(GetCurrentProcess(), &PerfS.meminfo, sizeof(PerfS.meminfo));
            PerfS.ramUsed = PerfS.meminfo.WorkingSetSize;
        #elif defined(__linux__)
            std::ifstream file("/proc/self/statm");
            size_t size;
            file >> size;
            long page_size_kb = sysconf(_SC_PAGE_SIZE);
            PerfS.ramUsed = size * page_size_kb;
        #endif
            if (PerfS.ramUsed >= game.Max_Ram*1024*1024) {
                if (game.ramHandle == 1) {
                    std::cerr << "Out of RAM! Changed RenderDistance by -1" << "\n";
                    if (Camera.RenderDistance > 1) {
                        Camera.RenderDistance -= 1;
                        ChunkGeneration::RemoveChunks();
                    }
                } else {
                    break;
                }
            }
        // Update Screen
            glfwSwapBuffers(window);
            PerfS.EntireTime = FrameTime.ElapsedMs();
            game.FPS = Fps.End();
    }
    GenerateChunk.Stop();
}

int main() {
    Game main;

    std::cout << "Initializing Settings:\n";
    main.Init_Settings("MyCraft/Assets/Settings.myc");
    if (Game::Init_Window()) return -1;
    std::cout << "Initializing Shaders:\n";
    Game::Init_Shader();
    std::cout << "Launching Game:\n";
    main.MainLoop();
    std::cout << "Cleaning:\n";
    Game::CleanUp();
    std::cout << "Safely Closed Game\n";
    return 0;
}