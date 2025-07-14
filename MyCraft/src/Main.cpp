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

int CHUNK_WIDTH = 32;
int CHUNK_HEIGHT = 16;
int CHUNK_DEPTH = 32;

const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fragColor;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
    fragColor = aColor;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
in vec3 fragColor;
//uniform vec3 myColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(fragColor, 1.0);
}
)glsl";

//bool IsChunkInFrontOfCamera(const glm::vec3& cameraPos, float yaw, float pitch, int chunkX, int chunkZ) {
//
//    glm::vec3 direction;
//    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
//    direction.y = sin(glm::radians(pitch));
//    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
//    direction = glm::normalize(direction);
//
//    glm::vec3 chunkCenter = glm::vec3(
//        chunkX * CHUNK_WIDTH + CHUNK_WIDTH / 2,
//        cameraPos.y,
//        chunkZ * CHUNK_DEPTH + CHUNK_DEPTH / 2
//    );
//
//    glm::vec3 toChunk = glm::normalize(chunkCenter - cameraPos);
//
//    float dot = glm::dot(direction, toChunk);
//
//    return dot > 0.3f;
//}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct camera {
    glm::vec3 Position = glm::vec3(0.0f, 5.0f, 5.0f);
    float Pitch = 0.0f;
    float Yaw = -90.0f;
    float Speed = 5.0f;
    float Sensitivity = 0.1f;
    bool FirstMouse = true;
    float LastX = 400, LastY = 300;

    bool operator!=(const camera& other) const {
        return Position != other.Position ||
               Pitch != other.Pitch ||
               Yaw != other.Yaw ||
               Speed != other.Speed ||
               Sensitivity != other.Sensitivity ||
               FirstMouse != other.FirstMouse ||
               LastX != other.LastX ||
               LastY != other.LastY;
    }
};

struct Game_Variables {
    int Render_Distance = 6;
    int VRamAlloc = 512; // In MB
    GLint sizeInBytes = 0;
    uint64_t Frame = 0;
    glm::ivec3 Last_Chunk;
    bool ChunkUpdated;
};

struct Entity {
    glm::vec3 Pos;
    glm::ivec3 Chunk;
    float Speed;
};

struct Block {
    int8_t id;
    bool transparent; // glass
    bool solid;       // Colisions
    uint8_t light;    // How Block is Lighted

    Block(uint8_t id = 0, bool transparent = false, bool solid = false, uint8_t light = 0)
        : id(id), transparent(transparent), solid(solid), light(light) {}
};

class Chunk {
private:
    std::vector<Block> blocks;
public:
    static const std::map<uint8_t, Block> BlockDefs;

    Chunk() : blocks(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH) {}

    int index(int x, int y, int z) const {
        return x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH;
    }

    const Block& get(int x, int y, int z) const {
        return blocks[index(x, y, z)];
    }

    void set(int x, int y, int z, const Block& block) {
        blocks[index(x, y, z)] = block;
    }

    void setID(int x, int y, int z, uint8_t id) {
        blocks[index(x, y, z)].id = id;
    }
};

const std::map<uint8_t, Block> Chunk::BlockDefs = {
    { 0, Block(0, false, false) }, // Air
    { 1, Block(1, false, true)  }, // Stone
    { 2, Block(2, false,  true)  }, // Grass
};

void Get_World_Chunk(int Chunk_x, int Chunk_z, std::map<std::pair<int, int>, Chunk>& World) {
    Chunk chunk;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            // Coordinates
            int worldX = Chunk_x * CHUNK_WIDTH + x;
            int worldZ = Chunk_z * CHUNK_DEPTH + z;

            // Sin terrain
            float height_f = 0.5f + 0.5f * std::sin(worldX * 0.3f) * std::cos(worldZ * 0.3f);
            int height = static_cast<int>(height_f * CHUNK_HEIGHT);

            for (int y = 0; y <= height; ++y) {
                chunk.setID(x,y,z,1);
            }
        }
    }

    World[{Chunk_x, Chunk_z}] = std::move(chunk);
}

void Generate_Chunks(const Entity& Player, int Render_Dist, std::map<std::pair<int, int>, Chunk>& World, camera &Camera) {
    glm::ivec3 Start_Chunk = Player.Chunk;

    for (int dx = -Render_Dist; dx <= Render_Dist; ++dx) {
        for (int dz = -Render_Dist; dz <= Render_Dist; ++dz) {
            int chunkX = Start_Chunk.x + dx;
            int chunkZ = Start_Chunk.z + dz;

            std::pair<int, int> key = {chunkX, chunkZ};

            if (World.find(key) == World.end()) {
                //if (IsChunkInFrontOfCamera(Player.Pos, Camera.Yaw, Camera.Pitch, chunkX, chunkZ)) {
                Get_World_Chunk(chunkX, chunkZ, World);
                //}
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

    auto pushTri = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color) {
        for (glm::vec3 v : {a, b, c}) {
            vertices.insert(vertices.end(), { v.x, v.y, v.z, color.r, color.g, color.b });
        }
    };

    // Colors soon uv cords
    glm::vec3 red     = {1.0f, 0.0f, 0.0f};
    glm::vec3 green   = {0.0f, 1.0f, 0.0f};
    glm::vec3 blue    = {0.0f, 0.0f, 1.0f};
    glm::vec3 yellow  = {1.0f, 1.0f, 0.0f};
    glm::vec3 cyan    = {0.0f, 1.0f, 1.0f};
    glm::vec3 magenta = {1.0f, 0.0f, 1.0f};

    // FRONT (z+)
    if (Localz + 1 >= CHUNK_DEPTH || chunk.get(Localx,Localy,Localz+1).id == 0) {
    pushTri(p001, p101, p111, red);
    pushTri(p001, p111, p011, red);
    }
    
    // BACK (z-)
    if ((Localz - 1 < 0) || chunk.get(Localx,Localy,Localz-1).id == 0) {
    pushTri(p100, p000, p010, green);
    pushTri(p100, p010, p110, green);
    }

    // LEFT (x-)
    if ((Localx - 1 < 0) || chunk.get(Localx-1,Localy,Localz).id == 0) {
    pushTri(p000, p001, p011, blue);
    pushTri(p000, p011, p010, blue);
    }

    // RIGHT (x+)
    if (Localx + 1 >= CHUNK_WIDTH || chunk.get(Localx+1,Localy,Localz).id == 0) {
    pushTri(p100, p101, p111, yellow);
    pushTri(p100, p111, p110, yellow);
    }

    // TOP (y+)
    if (Localy + 1 >= CHUNK_HEIGHT || chunk.get(Localx,Localy+1,Localz).id == 0) {
    pushTri(p010, p011, p111, cyan);
    pushTri(p010, p111, p110, cyan);
    }

    // BOTTOM (y-)
    if ((Localy - 1 < 0) || chunk.get(Localx,Localy-1,Localz).id == 0) {
    pushTri(p000, p100, p101, magenta);
    pushTri(p000, p101, p001, magenta);
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

void Input_Handler(camera &Camera, GLFWwindow* window, float deltaTime) {
    float velocity = Camera.Speed * deltaTime;
    glm::vec3 direction;
    direction.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction.y = sin(glm::radians(Camera.Pitch));
    direction.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    glm::vec3 front = glm::normalize(direction);
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0,1,0)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Camera.Position += front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Camera.Position -= front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Camera.Position -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Camera.Position += right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        Camera.Position.y += velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        Camera.Position.y -= velocity;
}

class FPS {
private:
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    int nbFrames = 0;
    float lastTime = 0.0f;

public:
    void Init() {
        lastFrame = glfwGetTime();
        lastTime = lastFrame;
        deltaTime = 0.0f;
        nbFrames = 0;
    }

    float Start() {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        return deltaTime;
    }

    void End() {
        float currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 2.0f) {
            std::cout << "FPS: " << nbFrames/2 << "\n";
            nbFrames = 0;
            lastTime = currentTime;
        }
    }

    float GetDeltaTime() const { return deltaTime; }
};

class Game {
private:
    SIZE_T ramUsed;
    PROCESS_MEMORY_COUNTERS meminfo;
    FPS Fps;
    camera Camera;
    Game_Variables game;
    GLFWwindow* window;
    GLuint VAO, VBO, ShaderProgram;
    std::vector<float> vertecies;
    Entity Player;
    std::map<std::pair<int, int>, Chunk> World;
    float DeltaTime;

public:
    bool Init_Window();
    void Init_Shader();
    void MainLoop();
    void CleanUp();
};

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

    window = glfwCreateWindow(800, 600, "MyCraft", nullptr, nullptr);
    if (!window) {
        std::cerr << "skill issue with GLFW!\n";
        glfwTerminate();
        return true;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-sync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Skill Issue of OpenGl and Glad!\n";
        return true;
    }

    glEnable(GL_DEPTH_TEST); // Not Working but maybe someday
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, &Camera);
    glfwSetCursorPosCallback(window, mouse_callback);

    game.Last_Chunk = glm::ivec3(999, 999, 999);
    
    return 0;
}

void Game::Init_Shader() {
    // Compilation of Shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &fragmentShaderSource, nullptr);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // aColor (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
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

    glUseProgram(ShaderProgram);
} 

void Game::MainLoop() {
    Fps.Init();
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
            
        // Main Engine -------------------------------------------------------------------
            
            DeltaTime = Fps.Start();

            Input_Handler(Camera, window, DeltaTime); // handle Camera Movment

        // Clearing
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MVP
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = GetViewMatrix(Camera);
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 1000.0f);
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

                    //GLuint colorLoc = glGetUniformLocation(ShaderProgram, "myColor");
                    //glUniform3f(colorLoc, 0.8f, 0.9f, 1.0f);

                    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &game.sizeInBytes);
                    GetProcessMemoryInfo(GetCurrentProcess(), &meminfo, sizeof(meminfo));
                    ramUsed = meminfo.WorkingSetSize;
                    
                    
                    std::cout << "Sending: #" << game.Frame << " Info: verts=" << vertecies.size() << " chunks=" << World.size() << " Buffer size B: " 
                        << (vertecies.size() * sizeof(float))/1048576 << "MB/"<< game.sizeInBytes/1048576 << "MB" << " Render Dist:" << game.Render_Distance << " Ram Used:" << ramUsed / 1024 / 1024 << "MB"
                        << " World usage:" << (vertecies.capacity() * sizeof(float)) / 1048576 << "MB" <<"\n";
                }
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertecies.size() / 6);
            }

            Fps.End();
        // Update Screen
            glfwSwapBuffers(window);
            glfwPollEvents();
    }
}

int main() {
    Game game;

    if (game.Init_Window()) return -1;
    
    game.Init_Shader();
    game.MainLoop();
    game.CleanUp();
    std::cout << "Safely Closed App";
    return 0;
}