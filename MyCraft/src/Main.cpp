#include "Main.hpp"
#define WIN32_LEAN_AND_MEAN
#include <ranges>
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
#include "Tick/Tick.hpp"
#include "Shader_Utils/Shader.hpp"
#include "World/Mesh/Mesh.hpp"

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    game_settings.width = width;
    game_settings.height = height;
    glViewport(0, 0, width, height);
}

void Game::MainLoop() {
    init_block_state();
    World_Map::World.reserve(512);
    World_Map::World.max_load_factor(0.5f);
    World_Map::Render_List.reserve(1024);
    game.TimeOfDay = 0.985;

    glfwGetWindowSize(window, &game_settings.width, &game_settings.height);
    selection.Init(SH.SelectionBox_Shader.Shader);
    Fps.Init();
    GenerateChunk.Start(game_settings.Generation_Threads);
    mesher.start(game_settings.Mesher_Threads);
    skybox.Create_SkyBox();

    while (!glfwWindowShouldClose(window)) {
        game.DeltaTime = Fps.Start();

        PerfS.EntireTime.Reset();
        if (game_settings.width == 0 || game_settings.height == 0) {
            glfwPollEvents();
            continue;
        }
        PerfS.pollevents.Reset();
        glfwPollEvents();
        PerfS.pollevents.Stop();

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
        const glm::mat4 proj = glm::perspective(glm::radians(FOV), aspectRatio, 0.1f, 10000.0f);
        const glm::mat4 invView = glm::inverse(view);
        const glm::mat4 invProj = glm::inverse(proj);

        //game.Seconds_elapsed += Fps.GetDeltaTime();
        //game.TimeOfDay += Fps.GetDeltaTime() / game_settings.DayCycleDuration;
        game.TimeOfDay = fmod(game.TimeOfDay, 1.0f);

        const float angle = game.TimeOfDay * glm::two_pi<float>();
        auto sunDir = glm::vec3(cos(angle), sin(angle), sin(angle) * 0.3f);

        sunDir = glm::normalize(sunDir);
        const float dayFactor = glm::clamp(sunDir.y * 0.5f + 0.5f, 0.0f, 1.0f);

        PerfS.skybox.Reset();
        skybox.Render_SkyBox(invProj, invView, sunDir);
        //skybox.Render_Clouds(invProj, invView, sunDir);
        PerfS.skybox.Stop();

        glUseProgram(SH.Solid_Shader_Blocks.Shader);
        Shader::Set_Int(SH.Solid_Shader_Blocks.Shader, "BaseTexture", 0);
        Shader::Set_Vec3(SH.Solid_Shader_Blocks.Shader, "ViewPos", Camera.Position);
        Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Model", model);
        Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "View", view);
        Shader::Set_Mat4(SH.Solid_Shader_Blocks.Shader, "Proj", proj);
        Shader::Set_Int(SH.Solid_Shader_Blocks.Shader, "RenderDist", Camera.RenderDistance);
        Shader::Set_Vec3(SH.Solid_Shader_Blocks.Shader, "Sun", sunDir);
        Shader::Set_Float(SH.Solid_Shader_Blocks.Shader, "dayfactor", dayFactor);

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
        // World Generation
        //-------------------------
        PerfS.remove.Reset();
        if (game.ChunkUpdated) {
            ChunkGeneration::RemoveChunks();
        }
        PerfS.remove.Stop();

        PerfS.chunk.Reset();
        if (game.ChunkUpdated) {
            if (game.World_Updates == 0) {
                GenerateChunk.LookForChunks();
            }
        }

        //-------------------------
        // World moving
        {
            std::lock_guard lock(GenerateChunk.ResultMutex);
            while (!GenerateChunk.ReadyChunks.empty()) {
                if (PerfS.chunk.ElapsedMs() > 3.0)
                    break;

                auto chunk = std::move(GenerateChunk.ReadyChunks.front());
                GenerateChunk.ReadyChunks.pop_front();

                const auto pos = std::pair{chunk->chunkX, chunk->chunkZ};
                Chunk* ptr = chunk.get();

                World_Map::World[pos] = std::move(chunk);

                mesher.pendingChunks.push_back(ptr);
                ptr->pending_mesh = true;

                GenerateChunk.GeneratingChunks.erase(pos);
            }
        }
        PerfS.chunk.Stop();

        //-------------------------
        // Tick Update
        //-------------------------
        PerfS.tick.Reset();
        while (game.Tick_Timer >= game.TickRate) {
            game.Tick_Timer -= game.TickRate;
            if (!game.ChunkUpdated) {
                Tick::Tick(movement, selection);
            }
        }
        PerfS.tick.Stop();


        //-------------------------
        // Deleting mesh
        PerfS.mesh.Reset();
        if (!World_Map::Render_List.empty()) {
            for (size_t i = World_Map::Render_List.size(); i-- > 0;) {
                auto& info = World_Map::Render_List[i];
                const int dist = std::max(std::abs(info.chunkX-Camera.Chunk.x), std::abs(info.chunkZ-Camera.Chunk.z));
                if (info.Delete == 5 || dist > Camera.RenderDistance+1) {

                    auto *chunk = World_Map::find_chunk(info.chunkX, info.chunkZ);
                    if (chunk) {
                        chunk->InRender = false;
                        chunk->has_mesh = false;
                    }

                    glDeleteBuffers(1, &info.vbo);
                    glDeleteVertexArrays(1, &info.vao);

                    if (i != World_Map::Render_List.size() - 1)
                        std::swap(World_Map::Render_List[i], World_Map::Render_List.back());

                    World_Map::Render_List.pop_back();
                }
            }
        }

        //-------------------------
        // Mesher
        //-------------------------
        {
            std::lock_guard lock(mesher.meshInMutex);
            for (const auto chunk : mesher.pendingChunks) {
                chunk->is_edge = World_Map::is_edge(chunk);
                chunk->pending_mesh = false;
                if (!chunk->has_terrain || chunk->is_edge || !chunk->DirtyFlag) continue;
                mesher.meshQueue.emplace(chunk->chunkX, chunk->chunkZ);
                chunk->in_mesher = true;
            }
            mesher.meshCV.notify_all();
        }
        mesher.pendingChunks.clear();

        while (true) {
            if (PerfS.mesh.ElapsedMs() > 3.0) break;
            mesh_t mesh;
            {
                std::lock_guard lock(mesher.meshOutMutex);
                if (mesher.meshOutQueue.empty()) break;
                mesh = std::move(mesher.meshOutQueue.front());
                mesher.meshOutQueue.pop_front();
            }

            if (mesh.R == result::Invalid_ptr) continue;
            const auto chunk = World_Map::find_chunk(mesh.chunkX, mesh.chunkZ);

            if (chunk) {
                if (mesh.R == result::Bad_Flags) {
                    chunk->in_mesher = false;
                    continue;
                }

                if (mesh.R == result::Missing_N) {
                    chunk->in_mesher = false;
                    mesher.pendingChunks.push_back(chunk);
                    chunk->pending_mesh = true;
                    continue;
                }
                if (chunk->InRender) {
                    chunk->InRender = false;
                    chunk->has_mesh = false;
                    auto it = std::ranges::find_if(World_Map::Render_List,
                                                     [&chunk](const World_Map::Render_Info &r){ return r.chunkX == chunk->chunkX && r.chunkZ == chunk->chunkZ; });

                    if (it != World_Map::Render_List.end()) {
                        glDeleteBuffers(1, &it->vbo);
                        glDeleteVertexArrays(1, &it->vao);
                        *it = World_Map::Render_List.back();
                        World_Map::Render_List.pop_back();
                    }
                }
                chunk->in_mesher = false;
                chunk->DirtyFlag = false;
                chunk->InRender = true;
                chunk->has_mesh = true;
                chunk->vao = 0; chunk->vbo = 0;
                chunk->Mesh.clear();
                chunk->Mesh = std::move(mesh.mesh);
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
                chunk->Mesh.clear();
                chunk->Mesh.shrink_to_fit();
            }
        }
        PerfS.mesh.Stop();

        //-------------------------
        // Drawing Mesh to Screen
        //-------------------------
        PerfS.render.Reset();
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        PerfS.Capacity = 0; PerfS.Mesh_Size = 0; PerfS.Triangles = 0; PerfS.Total_Triangles = 0;
        if (game_settings.width != 0 && game_settings.height != 0) {
            for (auto& info : World_Map::Render_List) {
                if (info.Delete > 0)
                    info.Delete++;

                const auto chunkMin = glm::vec3(info.chunkX * Chunk::WIDTH, 0, info.chunkZ * Chunk::DEPTH);
                const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk::WIDTH, Chunk::HEIGHT, Chunk::DEPTH);

                if (Frustum::IsAABBVisible(Frust, chunkMin, chunkMax)) {
                    Shader::Set_Vec3(SH.Solid_Shader_Blocks.Shader, "ChunkOffset", {info.chunkX * 16, 0, info.chunkZ * 16});
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
        glDisable(GL_CULL_FACE);

        PerfS.render.Stop();

        //-------------------------
        // GUI - My Own GUI Engine
        //-------------------------
        PerfS.gui.Reset();
        gui.backend.ResetFrame();
        gui.Update();
        gui.Generate();
        gui.backend.SendMesh();
        gui.backend.RenderFrame();
        PerfS.gui.Stop();

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
                if (Camera.RenderDistance > 2) {
                    Camera.RenderDistance -= 1;
                    ChunkGeneration::RemoveChunks();
                }
            } else {
                break;
            }
        }
        // --------------------------
        // Bloom
        PerfS.bloom.Reset();
        bloom.Extract(sceneTex);
        bloom.Blur();
        bloom.Combine(sceneTex);
        PerfS.bloom.Stop();

        // Update Screen
        glfwSwapBuffers(window);
        PerfS.EntireTime.Stop();
        game.FPS = Fps.End();
    }
    GenerateChunk.Stop();
    mesher.stop();
}

void Game::CleanUp() {
    glFinish();
    for (const auto &chunk: World_Map::World | std::views::values) {
        chunk->RemoveData();
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
    if (!main.Init_Settings("MyCraft/Assets/Settings.myc")) return 1;
    std::cout << "Initializing JSON files:\n";
    if (!main.Init_JSON()) return 2;
    if (main.Init_Window()) return 3;
    std::cout << "Initializing Shaders:\n";
    Game::Init_Shader();
    std::cout << "Launching Game:\n";
    main.MainLoop();
    std::cout << "Cleaning:\n";
    Game::CleanUp();
    std::cout << "Safely Closed Game\n";
    return 0;
}