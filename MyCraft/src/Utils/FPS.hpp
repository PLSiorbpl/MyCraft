#pragma once

class FPS {
private:
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    int nbFrames = 0;
    float lastTime = 0.0f;

public:
    void Init();
    float Start();
    void End();
    float GetDeltaTime() const;
};