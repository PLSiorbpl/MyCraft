#include "FPS.hpp"

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

int FPS::End() {
    float currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0f) {
        ToReturn = nbFrames;
        nbFrames = 0;
        lastTime = currentTime;
    }
    return ToReturn;
}

float FPS::GetDeltaTime() const { return deltaTime; }