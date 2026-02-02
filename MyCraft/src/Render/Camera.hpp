#pragma once
#include <glm/glm.hpp>

class camera {
public:
    glm::vec3 Position = glm::vec3(0.0f, 150.0f, 0.0f);
    float Pitch = 0.0f;
    float Yaw = -90.0f;
    bool Mouse_Visible = false;
    bool Can_Move = true;
    int ItemHeld = 1;
    int HotBarSlot = 0;
    
    float CameraDrag = 0.1f;
    float Speed = 0.10f; // 0.10f
    float SprintSpeed = 0.001f; // 0.01f
    float Gravity = 0.01f;
    float JumpStrength = 0.15f;
    float Sensitivity = 0.1f;
    bool Mode = false;
    bool crouching = false;

    bool FirstMouse = true;
    float LastX = 400, LastY = 300;
    glm::vec3 Vel = glm::vec3(0.0f,0.0f,0.0f);
    bool onGround = false;
    glm::ivec3 Chunk = glm::ivec3(0,0,0);
    int Place_CoolDown = 0;
    int Break_CoolDown = 0;
    bool Draw_Selection = false;
    int RenderDistance = 8;

    bool operator!=(const camera& other) const;
};