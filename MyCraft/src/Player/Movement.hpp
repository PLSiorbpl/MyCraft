#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "Breaking.hpp"

class Movement {
    private:
    Terrain_Action TAction;
    public:
    glm::vec3 direction;
    glm::vec2 Cos;
    glm::vec2 Sin;
    glm::vec3 testPos;

    void Init(Selection& Sel);
    void Input(Selection& Sel);

    static void Special_Keys();
    void TestColisions();

    static void Damp();

    static glm::mat4 GetViewMatrix();
};