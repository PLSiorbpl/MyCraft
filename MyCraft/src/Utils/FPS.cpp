#include "FPS.hpp"
#include <GLFW/glfw3.h>
#include <iostream>


void FPS::Init() {
    lastFrame = glfwGetTime();
    lastTime = lastFrame;
    deltaTime = 0.0f;
    nbFrames = 0;
}


float FPS::Start() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    return deltaTime;
}


void FPS::End() {
    float currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 2.0f) {
        std::cout << "FPS: " << nbFrames/2 << "\n";
        nbFrames = 0;
        lastTime = currentTime;
    }
}


float FPS::GetDeltaTime() const { return deltaTime; }