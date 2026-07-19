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

    void Init(GLFWwindow* window, Selection& Sel);
    void Input(GLFWwindow* window, Selection& Sel);

    static void Special_Keys(GLFWwindow* window);
    void TestColisions();

    static void Damp();

    static glm::mat4 GetViewMatrix();
};