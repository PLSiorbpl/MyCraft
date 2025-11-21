#include "Movement.hpp"

void Movement::Init(camera &Camera, GLFWwindow* window, const glm::ivec3 ChunkSize, colisions &Colisions, Selection& Sel) {
    direction = glm::vec3(0.0f);
    Cos.x = cos(glm::radians(Camera.Pitch));
    Cos.y = cos(glm::radians(Camera.Yaw));
    Sin.x = sin(glm::radians(Camera.Pitch));
    Sin.y = sin(glm::radians(Camera.Yaw));

    Input(window, Camera, ChunkSize, Sel);
    Special_Keys(window, Camera);
    TestColisions(Camera, ChunkSize, Colisions);
    Damp(Camera);

    if (Camera.Place_CoolDown > 0) Camera.Place_CoolDown -= 1;
    if (Camera.Break_CoolDown > 0) Camera.Break_CoolDown -= 1;
}

void Movement::Input(GLFWwindow* window, camera &Camera, glm::ivec3 ChunkSize, Selection& Sel) {
    //// Slipperiness
    //float slipperiness = 0.6f;
    //if (Camera.onGround) {
    //    Camera.Vel.x *= slipperiness;
    //    Camera.Vel.z *= slipperiness;
    //} else {
    //    // in air in Minecraft 0.91f
    //    Camera.Vel.x *= 0.91f;
    //    Camera.Vel.z *= 0.91f;
    //}
    glm::vec2 moveVec(0.0f, 0.0f);
    float speed = Camera.Speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed += Camera.SprintSpeed;

    // (x -> right, z -> forward) -> Sin/Cons
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveVec += glm::vec2(-Sin.y,  Cos.y);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveVec += glm::vec2( Sin.y, -Cos.y);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveVec += glm::vec2(-Cos.y, -Sin.y);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveVec += glm::vec2( Cos.y,  Sin.y);

    // Normalize Lenght
    float len = glm::length(moveVec);
    if (len > 1e-5f) {
        moveVec /= len; // Lenght = 1
        Camera.Vel.x = moveVec.x * speed;
        Camera.Vel.z = moveVec.y * speed;
    } else {
        Camera.Vel.x = 0.0f;
        Camera.Vel.z = 0.0f;
    }

    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && Camera.onGround) {
        Camera.Vel.y = Camera.JumpStrength;
        Camera.onGround = false;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) Camera.Vel.y = Camera.JumpStrength;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) Camera.Vel.y = -Camera.JumpStrength;

    // Mouse Actions
    TAction.RayCastBlock(Camera, ChunkSize, 0, 0, Sel);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) TAction.RayCastBlock(Camera, ChunkSize, 1, 0, Sel);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) TAction.RayCastBlock(Camera, ChunkSize, 2, 3, Sel);

    Camera.Vel.y = std::clamp(Camera.Vel.y, -0.5f, 0.2f);
}

void Movement::Special_Keys(GLFWwindow* window, camera &Camera) {
    bool currentState = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    if (currentState && !lastStateKey1) Camera.Mode = !Camera.Mode;
    lastStateKey1 = currentState;
}

void Movement::TestColisions(camera &Camera, const glm::ivec3 ChunkSize, colisions &Colisions) {
    const auto& World = World_Map::World;
    // get unstuck from block
    if (Colisions.isSolidAround(Camera.Position, ChunkSize) && !Camera.Mode) {
        Camera.Position.y += 1.0f;
    }

    if (!Camera.Mode) Camera.Vel.y -= Camera.Gravity;

    testPos = Camera.Position + glm::vec3(Camera.Vel.x, 0, 0);
    if (!Colisions.isSolidAround(testPos, ChunkSize) || Camera.Mode) {
        Camera.Position.x = testPos.x;
    } else if (Camera.Mode) {
        Camera.Vel.x = 0.0f;
    }

    testPos = Camera.Position + glm::vec3(0, Camera.Vel.y, 0);
    if (!Colisions.isSolidAround(testPos, ChunkSize) || Camera.Mode) {
        Camera.Position.y = testPos.y;
        Camera.onGround = false;
    } else {
        if (Camera.Vel.y < 0.0f) {
            Camera.onGround = true;
        }
        Camera.Vel.y = 0.0f;
    }

    testPos = Camera.Position + glm::vec3(0, 0, Camera.Vel.z);
    if (!Colisions.isSolidAround(testPos, ChunkSize) || Camera.Mode) {
        Camera.Position.z = testPos.z;
    } else {
        Camera.Vel.z = 0.0f;
    }

    if (Camera.Mode) Camera.Vel.y = 0.0f;
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

void Movement::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static camera* Camera = reinterpret_cast<camera*>(glfwGetWindowUserPointer(window));

    if (Camera->FirstMouse) {
        Camera->LastX = xpos;
        Camera->LastY = ypos;
        Camera->FirstMouse = false;
    }

    float xoffset = xpos - Camera->LastX;
    float yoffset = Camera->LastY - ypos;

    Camera->LastX = xpos;
    Camera->LastY = ypos;

    xoffset *= Camera->Sensitivity;
    yoffset *= Camera->Sensitivity;

    Camera->Yaw   += xoffset;
    Camera->Pitch += yoffset;

    Camera->Pitch = std::clamp(Camera->Pitch, -89.0f, 89.0f);
}

glm::mat4 Movement::GetViewMatrix(const camera& Camera) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction.y = sin(glm::radians(Camera.Pitch));
    direction.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));

    const glm::vec3 front = glm::normalize(direction);
    const glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0,1,0)));
    const glm::vec3 up    = glm::normalize(glm::cross(right, front));

    return glm::lookAt(Camera.Position, Camera.Position + front, up);
}