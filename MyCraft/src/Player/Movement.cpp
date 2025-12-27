#include "Movement.hpp"

void Movement::Init(GLFWwindow* window, const glm::ivec3 ChunkSize, colisions &Colisions, Selection& Sel) {
    direction = glm::vec3(0.0f);
    Cos.x = cos(glm::radians(Camera.Pitch));
    Cos.y = cos(glm::radians(Camera.Yaw));
    Sin.x = sin(glm::radians(Camera.Pitch));
    Sin.y = sin(glm::radians(Camera.Yaw));

    if (Camera.Can_Move) {
        Input(window, ChunkSize, Sel);
        Special_Keys(window);
        TestColisions(ChunkSize, Colisions);
        Damp();
        
        if (Camera.Place_CoolDown > 0) Camera.Place_CoolDown -= 1;
        if (Camera.Break_CoolDown > 0) Camera.Break_CoolDown -= 1;
    }
}

void Movement::Input(GLFWwindow* window, glm::ivec3 ChunkSize, Selection& Sel) {
    glm::vec2 moveVec(0.0f, 0.0f);
    float speed = Camera.Speed;
    if (In.keysState[GLFW_KEY_LEFT_SHIFT]) speed += Camera.SprintSpeed;

    // (x -> right, z -> forward) -> Sin/Cons
    if (In.keysState[GLFW_KEY_W]) moveVec += glm::vec2( Cos.y,  Sin.y);
    if (In.keysState[GLFW_KEY_A]) moveVec += glm::vec2( Sin.y, -Cos.y);
    if (In.keysState[GLFW_KEY_S]) moveVec += glm::vec2(-Cos.y, -Sin.y);
    if (In.keysState[GLFW_KEY_D]) moveVec += glm::vec2(-Sin.y,  Cos.y);
    
    // Jump
    if (In.keysState[GLFW_KEY_SPACE] && Camera.onGround) {
        Camera.Vel.y = Camera.JumpStrength;
        Camera.onGround = false;
    }
    if (In.keysState[GLFW_KEY_E]) Camera.Vel.y = Camera.JumpStrength;
    if (In.keysState[GLFW_KEY_LEFT_CONTROL]) Camera.Vel.y = -Camera.JumpStrength;

    // Mouse Actions
    TAction.RayCastBlock(Camera, ChunkSize, 0, 0, Sel); // Selection Box
    if (In.MouseState[GLFW_MOUSE_BUTTON_1]) TAction.RayCastBlock(Camera, ChunkSize, 1, 0, Sel); // Break
    if (In.MouseState[GLFW_MOUSE_BUTTON_2]) TAction.RayCastBlock(Camera, ChunkSize, 2, Camera.ItemHeld, Sel); // Place

    // Normalize Lenght
    float len = glm::length(moveVec);
    if (len > 1e-5f) {
        moveVec /= len; // Lenght = 1
        Camera.Vel.x = moveVec.x * speed;
        Camera.Vel.z = moveVec.y * speed;
    }

    Camera.Vel.y = std::clamp(Camera.Vel.y, -0.5f, 0.2f);
}

void Movement::Special_Keys(GLFWwindow* window) {
    Camera.Mode = In.keysToggle[GLFW_KEY_1] ? true : false;
}

void Movement::TestColisions(const glm::ivec3 ChunkSize, colisions &Colisions) {
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

void Movement::Damp() {
    auto damp = [&](float& v) {
        const float damping = Camera.onGround ? 0.6f : 0.96f;
        v *= damping;
    };
    damp(Camera.Vel.x);
    damp(Camera.Vel.z);
}

glm::mat4 Movement::GetViewMatrix() {
    const auto direction = glm::vec3(
        cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch)),
        sin(glm::radians(Camera.Pitch)),
        sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch))
    );

    const glm::vec3 front = glm::normalize(direction);
    const glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0,1,0)));
    const glm::vec3 up    = glm::normalize(glm::cross(right, front));

    return glm::lookAt(Camera.Position, Camera.Position + front, up);
}