#pragma once
#include <glm/glm.hpp>

class camera {
public:
    glm::vec3 Position = glm::vec3(0.0f, 100.0f, 0.0f);
    float CameraDrag = 0.1f;
    float Pitch = 0.0f;
    float Yaw = -90.0f;
    float Speed = 0.10f;
    float Sensitivity = 0.1f;
    bool FirstMouse = true;
    float LastX = 400, LastY = 300;
    glm::vec3 Vel = glm::vec3(0.0f,0.0f,0.0f);
    bool onGround = false;
    float Gravity = 0.01f;
    float JumpStrength = 0.15f; // 0.15f;
    glm::ivec3 Chunk = glm::ivec3(0,0,0);
    int RenderDistance;

    bool operator!=(const camera& other);
};