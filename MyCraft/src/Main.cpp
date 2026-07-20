#include "Main.hpp"
#define WIN32_LEAN_AND_MEAN
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#if defined(_WIN32) // Windows
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__) // Linux
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/sysinfo.h>
#endif

#include "Render/Frustum.hpp"
#include "World/Generation.hpp"
#include "World/Mesh.hpp"

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    const auto* ctx = static_cast<window_context*>(
        glfwGetWindowUserPointer(window)
    );
    ctx->game_settings->width = width;
    ctx->game_settings->height = height;
    glViewport(0, 0, width, height);
}

window_context Game::ctx = {&Camera, &game_settings};

void Tick_Update(GLFWwindow* window, Movement &movement, Selection &Sel) {
    movement.Init(window, Sel);
}

void Game::MainLoop() {
    init_block_state();
    World_Map::World.reserve(512);
    World_Map::World.max_load_factor(0.5f);
    World_Map::Mesh_Queue.reserve(256);
    World_Map::Render_List.reserve(1024);


    ChunkGeneration GenerateChunk;
    glfwGetWindowSize(window, &game_settings.width, &game_settings.height);
    selection.Init(SH.SelectionBox_Shader.Shader);
    Fps.Init();
    GenerateChunk.Start(game_settings.Generation_Threads);
    skybox.Create_SkyBox();

    while (!glfwWindowShouldClose(window)) {

        game.DeltaTime = Fps.Start();
        FrameTime.Reset();
        if (game_settings.width == 0 || game_settings.height == 0) {
            glfwPollEvents();
            continue;
        }
        glfwPollEvents();

        //-------------------------
        // Clearing Screen
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        const glm::mat4 invView = glm::inverse(view);
        const glm::mat4 invProj = glm::inverse(proj);

        game.TimeOfDay += Fps.GetDeltaTime() / game_settings.DayCycleDuration;
        game.TimeOfDay = fmod(game.TimeOfDay, 1.0f);

        const float angle = game.TimeOfDay * glm::two_pi<float>();

        auto sunDir = glm::vec3(
            cos(angle),
            sin(angle),
            sin(angle) * 0.3f
        );

        sunDir = glm::normalize(sunDir);

        skybox.Render_SkyBox(invProj, invView, sunDir);

        glUseProgram(SH.Solid_Shader_Blocks.Shader);
        Shader::Set_Int(SH.Solid_Shader_Blocks.Shader, "BaseTexture", 0);
        Shader::Set_Vec3(SH.Solid_Shader_Blocks.Shader, "ViewPos", Camera.Position);
        Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Model", model);
        Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "View", view);
        Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Proj", proj);
        Shader::Set_Int(SH.Solid_Shader_Blocks.Shader, "RenderDist", Camera.RenderDistance);
        Shader::Set_Vec3(SH.Solid_Shader_Blocks.Shader, "Sun", sunDir);

        const Frustum::Frust Frust = Frustum::ExtractFrustum(proj*view);

        Camera.Chunk.x = static_cast<int>(std::floor(Camera.Position.x / static_cast<float>(Chunk::WIDTH)));
        Camera.Chunk.y = 0;
        Camera.Chunk.z = static_cast<int>(std::floor(Camera.Position.z / static_cast<float>(Chunk::DEPTH)));
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
        //-------------------------
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
        //-------------------------
        game.Updates = 0;
        time.Reset();
        for (Chunk* chunk : World_Map::Mesh_Queue) {
            if (!chunk->has_terrain || chunk->is_edge || !chunk->DirtyFlag || chunk->InRender)
                continue;
            if (game.Updates >= game.Mesh_Updates)
                break;

            if (World_Map::find_chunk(chunk->chunkX, chunk->chunkZ + 1) == nullptr) continue;
            if (World_Map::find_chunk(chunk->chunkX, chunk->chunkZ - 1) == nullptr) continue;
            if (World_Map::find_chunk(chunk->chunkX + 1, chunk->chunkZ) == nullptr) continue;
            if (World_Map::find_chunk(chunk->chunkX - 1, chunk->chunkZ) == nullptr) continue;

            if (!World_Map::find_chunk(chunk->chunkX, chunk->chunkZ + 1)->has_terrain) continue;
            if (!World_Map::find_chunk(chunk->chunkX, chunk->chunkZ - 1)->has_terrain) continue;
            if (!World_Map::find_chunk(chunk->chunkX + 1, chunk->chunkZ)->has_terrain) continue;
            if (!World_Map::find_chunk(chunk->chunkX - 1, chunk->chunkZ)->has_terrain) continue;

            const auto chunkMin = glm::vec3(chunk->chunkX * Chunk::WIDTH, 0, chunk->chunkZ * Chunk::DEPTH);
            const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk::WIDTH, Chunk::HEIGHT, Chunk::DEPTH);

            const bool visible = Frustum::IsAABBVisible(Frust, chunkMin, chunkMax);
            // Normal Updates
            if (visible && game.Updates < game.Mesh_Updates) {
                chunk->Mesh.clear();
                Mesh::GenerateMesh(*chunk);
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
                chunk->has_mesh = true;
                chunk->InRender = true;
                game.Updates++;
                continue;
            }

            // Lazy Updates
            if (!visible && game.Updates < game.Lazy_Mesh_Updates) {
                chunk->Mesh.clear();
                Mesh::GenerateMesh(*chunk);
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
                chunk->has_mesh = true;
                chunk->InRender = true;
                game.Updates++;
            }
        }
        // Delete already done chunks
        if (!World_Map::Mesh_Queue.empty()) {
            for (size_t i = World_Map::Mesh_Queue.size(); i-- > 0;) {
                Chunk* chunk = World_Map::Mesh_Queue[i];

                if (!chunk->has_mesh)
                    continue;

                if (i != World_Map::Mesh_Queue.size() - 1)
                    std::swap(World_Map::Mesh_Queue[i], World_Map::Mesh_Queue.back());
                World_Map::Mesh_Queue.pop_back();
            }
        }
        PerfS.mesh = time.ElapsedMs();
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
                }

                const auto chunkMin = glm::vec3(info.chunkX * Chunk::WIDTH, 0, info.chunkZ * Chunk::DEPTH);
                const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk::WIDTH, Chunk::HEIGHT, Chunk::DEPTH);

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
        // --------------------------
        // Bloom
        bloom.Extract(sceneTex);
        bloom.Blur();
        bloom.Combine(sceneTex);

        // Update Screen
        glfwSwapBuffers(window);
        PerfS.EntireTime = FrameTime.ElapsedMs();
        game.FPS = Fps.End();
    }
    GenerateChunk.Stop();
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

int main() {
    Game main;

    std::cout << "Initializing Settings:\n";
    main.Init_Settings("MyCraft/Assets/Settings.myc");
    if (main.Init_Window()) return -1;
    std::cout << "Initializing Shaders:\n";
    Game::Init_Shader();
    std::cout << "Launching Game:\n";
    main.MainLoop();
    std::cout << "Cleaning:\n";
    Game::CleanUp();
    std::cout << "Safely Closed Game\n";
    return 0;
}