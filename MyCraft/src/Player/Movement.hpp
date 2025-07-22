#pragma once
#include <glm/glm.hpp>
#include "Render/Camera.hpp"
#include "World/Chunk.hpp"
#include "Colisions.hpp"
#include <GLFW/glfw3.h>
#include <map>
#include <algorithm>

class Movement {
    public:
    //colisions Colisions;
    glm::vec3 direction;
    glm::vec2 Cos;
    glm::vec2 Sin;
    glm::vec3 testPos;

    void Init(camera &Camera);
    void Input(GLFWwindow* window, camera &Camera);
    void TestColisions(camera &Camera, std::map<std::pair<int, int>, Chunk>& World, glm::vec3 ChunkSize, colisions &Colisions);
    void Damp(camera &Camera);
};