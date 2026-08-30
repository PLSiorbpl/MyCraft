#pragma once
#include <glm/glm.hpp>

class Entity {
public:
    virtual ~Entity() = default;

    int ID;
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Pitch;
    float Yaw;

    virtual void Tick() = 0;
};