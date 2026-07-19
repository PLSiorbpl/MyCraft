#include "Main.hpp"
#include <iostream>

#include "Utils/InputManager.hpp"

void Game::Init_Settings(const std::string& Path) {
    Settings.Load_Settings(Path);

    // General Options:
    Camera.RenderDistance = Settings.Get<int>("Render Distance", 2);
    game_settings.Generation_Threads = Settings.Get<unsigned int>("Generation Threads", 2);
    game_settings.DayCycleDuration = Settings.Get<unsigned int>("Day Cycle Duration", 600);

    video_settings.Blur_Scale = Settings.Get<unsigned int>("Blur Texture Scale", 4);
    video_settings.Blur_Passes = Settings.Get<unsigned int>("Blur Passes", 5);
    video_settings.Extract_Threshold = Settings.Get<float>("Extraction Threshold", 1.0f);
    video_settings.Exposure = Settings.Get<float>("Exposure", 1.0f);

    game.Max_Ram = Settings.Get<int>("RAM", 2048);
    game.V_Sync = Settings.Get<int>("V-Sync", 0);

    game.Mesh_Updates = Settings.Get<int>("Mesh Updates", 0);
    game.Lazy_Mesh_Updates = Settings.Get<int>("Lazy Mesh Updates", 0);
    game.ramHandle = Settings.Get<int>("Out Of ram", 1);

    // World Generation:
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

    bloom.Initialize(game_settings.width, game_settings.height);

    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // HDR texture
    glGenTextures(1, &sceneTex);
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, game_settings.width, game_settings.height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Scene FBO ERROR\n";

    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, game_settings.width, game_settings.height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return false;
}

void Game::Init_Shader() {
    Shader::Init_Shader();
}