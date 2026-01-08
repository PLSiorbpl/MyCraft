#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <unordered_map>

#include "Utils/Globals.hpp"
#include "Utils/InputManager.hpp"
#include "World/Chunk.hpp"
#include "Colisions.hpp"
#include "World/World.hpp"
#include "Breaking.hpp"

class Movement {
    private:
    Terrain_Action TAction;
    public:
    glm::vec3 direction;
    glm::vec2 Cos;
    glm::vec2 Sin;
    glm::vec3 testPos;

    void Init(GLFWwindow* window, glm::ivec3 ChunkSize, Selection& Sel);
    void Input(GLFWwindow* window, glm::ivec3 ChunkSize, Selection& Sel);

    static void Special_Keys(GLFWwindow* window);
    void TestColisions(glm::ivec3 ChunkSize);

    static void Damp();

    static glm::mat4 GetViewMatrix();
};