#include "Movement.hpp"

void Movement::Init(camera &Camera) {
    direction = glm::vec3(0.0f);
    Cos.x = cos(glm::radians(Camera.Pitch));
    Cos.y = cos(glm::radians(Camera.Yaw));
    Sin.x = sin(glm::radians(Camera.Pitch));
    Sin.y = sin(glm::radians(Camera.Yaw));
}

void Movement::Input(GLFWwindow* window, camera &Camera) {
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        Camera.Vel.x += -Camera.Speed * Sin.y;
        Camera.Vel.z +=  Camera.Speed * Cos.y;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        Camera.Vel.x +=  Camera.Speed * Sin.y;
        Camera.Vel.z += -Camera.Speed * Cos.y;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        Camera.Vel.x += -Camera.Speed * Cos.y;
        Camera.Vel.z += -Camera.Speed * Sin.y;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        Camera.Vel.x +=  Camera.Speed * Cos.y;
        Camera.Vel.z +=  Camera.Speed * Sin.y;
    }

    Camera.Vel.y -= Camera.Gravity;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && Camera.onGround) {
        Camera.Vel.y = Camera.JumpStrength;
        Camera.onGround = false;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        Camera.Vel.y = Camera.JumpStrength;
    }

    Camera.Vel.x = std::clamp(Camera.Vel.x, -0.1f, 0.1f);
    Camera.Vel.z = std::clamp(Camera.Vel.z, -0.1f, 0.1f);
    Camera.Vel.y = std::clamp(Camera.Vel.y, -0.2f, 0.2f);
}

void Movement::TestColisions(camera &Camera, std::map<std::pair<int, int>, Chunk>& World, glm::vec3 ChunkSize, colisions &Colisions) {
    // get unstuck from block
    if (Colisions.isSolidAt(Camera.Position, World, ChunkSize)) {
        Camera.Position.y += 1.0f;
    }

    testPos = Camera.Position + glm::vec3(Camera.Vel.x, 0, 0);
    if (!Colisions.isSolidAround(testPos, World, ChunkSize)) {
        Camera.Position.x = testPos.x;
    } else {
        Camera.Vel.x = 0.0f;
    }

    testPos = Camera.Position + glm::vec3(0, Camera.Vel.y, 0);
    if (!Colisions.isSolidAround(testPos, World, ChunkSize)) {
        Camera.Position.y = testPos.y;
        Camera.onGround = false;
    } else {
        if (Camera.Vel.y < 0.0f) {
            Camera.onGround = true;
        }
        Camera.Vel.y = 0.0f;
    }

    testPos = Camera.Position + glm::vec3(0, 0, Camera.Vel.z);
    if (!Colisions.isSolidAround(testPos, World, ChunkSize)) {
        Camera.Position.z = testPos.z;
    } else {
        Camera.Vel.z = 0.0f;
    }
}

void Movement::Damp(camera &Camera) {
    auto damp = [&](float& v) {
        if (v > Camera.CameraDrag)
            v -= Camera.CameraDrag;
        else if (v < -Camera.CameraDrag)
            v += Camera.CameraDrag;
        else
            v = 0;
    };
    damp(Camera.Vel.x);
    damp(Camera.Vel.z);
}