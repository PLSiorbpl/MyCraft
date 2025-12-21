#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cinttypes>
#if defined(_WIN32) // Windows
    #include <windows.h>
    #include <psapi.h>
#endif

#include "Render/Camera.hpp"

struct Game_Variables {
    int MenuId = 0;
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
    float DeltaTime;

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

struct Game_Settings {
    int Gui_Update_rate;
    int width, height;
};

struct PerfStats {
    // Times in ms
    double chunk = 0;
    double mesh = 0;
    double render = 0;
    double remove = 0;
    double tick = 0;
    double gui = 0;
    double EntireTime = 0;
    // OpenGL Version
    int major = 0, minor = 0;
    bool isModernGL;
    // Rendering Stats
    uint64_t Triangles;
    uint64_t Total_Triangles;
    size_t Capacity;
    size_t Mesh_Size;
    // Os specific Ram stats
    #if defined(_WIN32) // Windows
        PROCESS_MEMORY_COUNTERS meminfo;
        SIZE_T ramUsed;
    #elif defined(__linux__) // Linux
        size_t ramUsed;
    #endif
};

struct Sh {
    GLuint Shader;
    unsigned int Texture0;
    unsigned int Texture1;
    unsigned int Texture2;
    unsigned int Texture3;
};

struct Shaders {
    Sh Solid_Shader_Blocks;
    Sh General_Gui_Shader;
    Sh SelectionBox_Shader;
};

extern GLFWwindow* window;
extern camera Camera;
extern Game_Variables game;
extern Game_Settings game_settings;
extern PerfStats PerfS;
extern Shaders SH;