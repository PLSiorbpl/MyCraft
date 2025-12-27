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
    
    //#define _CRTDBG_MAP_ALLOC
    //#include <crtdbg.h>
#elif defined(__linux__) // Linux
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/sysinfo.h>
#endif
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION

// My Files
#include "GUI/Gui.hpp"

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
//#include "World/Terrain.hpp"
#include "World/Generation.hpp"
#include "World/World.hpp"

glm::ivec3 Chunk_Size = glm::ivec3(16);

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    glViewport(0, 0, width, height);
}

class Game {
private:
    Timer time;
    Timer FrameTime;
    FPS Fps;
    Movement movement;
    colisions Colisions;
    Fun fun;
    Shader shader;
    Mesh mesh;
    Gui gui;
    Frustum frustum;
    Selection selection;
public:
    Settings_Loader Settings;

    static bool Init_Window();
    void Init_Shader() const;
    void MainLoop();
    static void CleanUp();
    void Init_Settings(const std::string& Path);
};

void Game::Init_Settings(const std::string& Path) {
    Settings.Load_Settings(Path);

    Camera.RenderDistance = Settings.Get<int>("Render Distance", 0);
    game.Max_Ram = Settings.Get<int>("RAM", 0);
    game.ramHandle = Settings.Get<int>("Out Of ram", 0);
    Chunk_Size.x = Settings.Get<int>("Chunk Width", 0);
    Chunk_Size.y = Settings.Get<int>("Chunk Height", 0);
    Chunk_Size.z = Settings.Get<int>("Chunk Depth", 0);
    game.V_Sync = Settings.Get<int>("V-Sync", 0);
    game.TickRate = 1.0f / Settings.Get<float>("Tick Rate", 0.0f);
    game.FOV = Settings.Get<float>("FOV", 0);
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
    glFinish();
    for (auto& [key, chunk] : World_Map::World) {
        chunk.RemoveData();
    }
    //ImGui_ImplOpenGL3_Shutdown();
    //ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();
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
        std::cerr << "You Dont Support OpenGl 4.6 Trying 3.3...\n";

        // Fallback to 3.3 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(game_settings.width, game_settings.height, "MyCraft", monitor, nullptr);

        if (!window) {
            std::cerr << "You Dont Support OpenGl 3.3  Get New Card Or Update Drivers\n";
            glfwTerminate();
            return false;
        }
    }
    //glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowPos(window, 0, 0);
    //glfwSetWindowSize(window, game_settings.width, game_settings.height);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(game.V_Sync); // V-sync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Skill Issue of OpenGl and Glad!\n";
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwSetWindowUserPointer(window, &Camera);
    glfwSetMouseButtonCallback(window, InputManager::Mouse_Key_Callback);
    glfwSetKeyCallback(window, InputManager::Key_Callback);
    glfwSetScrollCallback(window, InputManager::Scroll_Callback);
    glfwSetCursorPosCallback(window, InputManager::Mouse_Callback);

    //ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //ImGui::StyleColorsDark();

    //ImGui_ImplGlfw_InitForOpenGL(window, true);
    //ImGui_ImplOpenGL3_Init("#version 330");

    game.Last_Chunk = glm::ivec3(999, 999, 999);

    glGetIntegerv(GL_MAJOR_VERSION, &PerfS.major);
    glGetIntegerv(GL_MINOR_VERSION, &PerfS.minor);

    PerfS.isModernGL = (PerfS.major >= 4);
    
    return false;
}

void Game::Init_Shader() const {
    shader.Init_Shader();
}

void Tick_Update(GLFWwindow* window, Movement &movement, colisions &Colisions, Selection &Sel) {
    movement.Init(window, Chunk_Size, Colisions, Sel);
}

void Game::MainLoop() {
    ChunkGeneration GenerateChunk(game.Seed, game.basefreq, game.baseamp, game.oct, game.addfreq, game.addamp, game.biomefreq, game.biomemult, game.biomebase, game.biomepower);
    glfwGetWindowSize(window, &game_settings.width, &game_settings.height);
    selection.Init(SH.SelectionBox_Shader.Shader);
    Fps.Init();
    while (!glfwWindowShouldClose(window)) {

            game.DeltaTime = Fps.Start();
            FrameTime.Reset();
            glfwGetWindowSize(window, &game_settings.width, &game_settings.height);
            if (game_settings.width == 0 || game_settings.height == 0) {
                glfwPollEvents();
                continue;
            }
            
        // -------------------------------------------------------------------------------
        // Main Engine
        // -------------------------------------------------------------------------------
            //-------------------------
            // Uniforms
            //-------------------------
            const float aspectRatio = static_cast<float>(game_settings.width) / static_cast<float>(std::max(game_settings.height, 1));
            const float FOV = fun.ConvertHorizontalFovToVertical(game.FOV, aspectRatio);

            static constexpr auto model = glm::mat4(1.0f);
            const glm::mat4 view = movement.GetViewMatrix();
            const glm::mat4 proj = glm::perspective(glm::radians(FOV), aspectRatio, 0.1f, 2000.0f);

            glUseProgram(SH.Solid_Shader_Blocks.Shader);
            //glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_2D, SH.Solid_Shader_Blocks.Texture0);
            shader.Set_Int(SH.Solid_Shader_Blocks.Shader, "BaseTexture", 0);
            shader.Set_Vec3(SH.Solid_Shader_Blocks.Shader, "ViewPos", Camera.Position);
            shader.Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Model", model);
            shader.Set_Mat4(SH.Solid_Shader_Blocks.Shader, "View", view);
            shader.Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Proj", proj);
            shader.Set_Int(SH.Solid_Shader_Blocks.Shader, "RenderDist", Camera.RenderDistance);
    
            const Frustum::Frust Frust = frustum.ExtractFrustum(proj*view);
    
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
                    Tick_Update(window, movement, Colisions, selection);
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
            
                const bool visible = frustum.IsAABBVisible(Frust, chunkMin, chunkMax);
                // Normal Updates
                if (visible && game.Updates < game.Mesh_Updates) {
                    chunk->Mesh.clear();
                    mesh.GenerateMesh(*chunk, chunk->Mesh, chunk->chunkX, chunk->chunkZ, Chunk_Size, Camera.RenderDistance);
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
                    chunk->Mesh.clear();
                    mesh.GenerateMesh(*chunk, chunk->Mesh, chunk->chunkX, chunk->chunkZ, Chunk_Size, Camera.RenderDistance);
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
            PerfS.mesh = time.ElapsedMs();
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

        //-------------------------
        // Clearing Screen
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //-------------------------
        // World/Mesh Generation
        //-------------------------
            if (game.ChunkUpdated) {
                if (game.World_Updates == 0) {
                    time.Reset();
                    GenerateChunk.GenerateChunks(Chunk_Size);
                    PerfS.chunk = time.ElapsedMs();
                }
                time.Reset();
                GenerateChunk.RemoveChunks();
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

                const glm::vec3 chunkMin = glm::vec3(info.chunkX * Chunk_Size.x, 0, info.chunkZ * Chunk_Size.z);
                const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk_Size);

                if (frustum.IsAABBVisible(Frust, chunkMin, chunkMax)) {
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
            shader.Set_Mat4(SH.SelectionBox_Shader.Shader, "MVP", MVP);
            glLineWidth(1.0f);
            glDrawArrays(GL_LINES, 0, 24);
        }
        }
        PerfS.render = time.ElapsedMs();
        
        //-------------------------
        // GUI - My Own GUI Engine
        //-------------------------
        time.Reset();
        glUseProgram(SH.General_Gui_Shader.Shader);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, SH.General_Gui_Shader.Texture1);
        shader.Set_Int(SH.General_Gui_Shader.Shader, "BaseTexture", 0);
        shader.Set_Int(SH.General_Gui_Shader.Shader, "GuiTexture", 1);
        shader.Set_Int(SH.General_Gui_Shader.Shader, "FontTexture", 2);

        int guiScaleX = game_settings.width  / 320;
        int guiScaleY = game_settings.height / 240;

        int guiScale = std::min(guiScaleX, guiScaleY);
        if (guiScale < 1)
            guiScale = 1;

        GLenum err = glGetError();

        if (game.Frame % game_settings.Gui_Update_rate == 0) {
            //-------------------------
            // Update Gui
            gui.Generate(game_settings.width/guiScale, game_settings.height/guiScale, guiScale);
        }
        //-------------------------
        // Render MyGui
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(game_settings.width)/static_cast<float>(guiScale),
            static_cast<float>(game_settings.height)/static_cast<float>(guiScale), 0.0f, -1.0f, 1.0f);
        shader.Set_Mat4(SH.General_Gui_Shader.Shader, "Model", model);
        shader.Set_Mat4(SH.General_Gui_Shader.Shader, "Projection", projection);

        gui.Draw();
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
            if (err == GL_OUT_OF_MEMORY || PerfS.ramUsed >= game.Max_Ram*1024*1024) {
                if (game.ramHandle == 1) {
                    std::cerr << "Out of VRAM! Changed RenderDistance by -1" << "\n";
                    if (Camera.RenderDistance > 1) {
                        Camera.RenderDistance -= 1;
                        GenerateChunk.RemoveChunks();
                    }
                } else {
                    break;
                }
            }
            PerfS.EntireTime = FrameTime.ElapsedMs();
            game.FPS = Fps.End();
        // Update Screen
            glfwSwapBuffers(window);
            glfwPollEvents();
    }
}

int main() {
    Game main;

    std::cout << "Initializing Settings:\n";
    main.Init_Settings("MyCraft/Assets/Settings.myc");
    if (Game::Init_Window()) return -1;
    std::cout << "Initializing Shaders:\n";
    main.Init_Shader();
    std::cout << "Launching Game:\n";
    main.MainLoop();
    std::cout << "Cleaning:\n";
    Game::CleanUp();
    std::cout << "Safely Closed Game";
    return 0;
}