#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <unordered_map>
#include "Render/Camera.hpp"
#include "World/Chunk.hpp"
#include "Colisions.hpp"
#include "World/World.hpp"

class Movement {
    public:
    //colisions Colisions;
    glm::vec3 direction;
    glm::vec2 Cos;
    glm::vec2 Sin;
    glm::vec3 testPos;
    bool lastStateKey1 = false;

    void Init(camera &Camera, GLFWwindow* window, const glm::ivec3 ChunkSize, colisions &Colisions);
    void Input(GLFWwindow* window, camera &Camera);
    void Special_Keys(GLFWwindow* window, camera &Camera);
    void TestColisions(camera &Camera, const glm::ivec3 ChunkSize, colisions &Colisions);
    void Damp(camera &Camera);

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    glm::mat4 GetViewMatrix(const camera &Camera);
};