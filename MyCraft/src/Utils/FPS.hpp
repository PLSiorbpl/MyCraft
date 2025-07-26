#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

class FPS {
private:
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    int nbFrames = 0;
    float lastTime = 0.0f;
    int ToReturn = 0;

public:
    void Init();
    float Start();
    int End();
    float GetDeltaTime() const;
};