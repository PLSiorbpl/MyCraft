#pragma once
#include <glad/glad.h>

// My files
#include "GUI/Gui.hpp"

#include "Player/Movement.hpp"

#include "Render/SkyBox/SkyBox.hpp"
#include "Render/Bloom/Bloom.hpp"
#include "Utils/FPS.hpp"

#include "Utils/Settings.hpp"

class Game {
private:
    FPS Fps;
    Movement movement = {};
    Gui gui = {};
    Selection selection = {};
    SkyBox skybox;
    Bloom bloom;
    GLuint sceneFBO;
    GLuint sceneTex;
public:
    Settings_Loader Settings;

    bool Init_Window();

    static void Init_Shader();
    void MainLoop();
    static void CleanUp();
    bool Init_Settings(const std::string& Path);
    bool Init_JSON();
};

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height);