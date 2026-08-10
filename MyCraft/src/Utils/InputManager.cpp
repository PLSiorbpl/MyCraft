#include "InputManager.hpp"
#include "Utils/Globals.hpp"
#include "Render/Camera.hpp"
#include <algorithm>

bool InputManager::keysToggle[512] = {false};
bool InputManager::keysState[512] = {false};
bool InputManager::MouseState[8] = {false};
bool InputManager::MouseToggle[8] = {false};
float InputManager::MouseX = 0.0f;
float InputManager::MouseY = 0.0f;
int InputManager::ScrollX = 0;
int InputManager::ScrollY = 0;
std::deque<char> InputManager::charBuffer;
bool InputManager::MouseVisible = false;
bool InputManager::InputActive = false;

void InputManager::Set_Mouse_Visiblility(bool visible) {
    if (MouseVisible == visible) return;

    if (visible) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        MouseVisible = true;
        Camera.Mouse_Visible = true;
        MouseX = game_settings.width/2; MouseY = game_settings.height/2;
        glfwSetCursorPos(window, MouseX, MouseY);
        Camera.LastX = MouseX; Camera.LastY = MouseY;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        MouseVisible = false;
        Camera.Mouse_Visible = false;
        MouseX = game_settings.width/2; MouseY = game_settings.height/2;
        glfwSetCursorPos(window, MouseX, MouseY);
        Camera.LastX = MouseX; Camera.LastY = MouseY;
    }
}

void InputManager::Key_Callback(GLFWwindow *window_, int key, int scancode, int action, int mods) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        keysState[key] = (action != GLFW_RELEASE);

        if (action == GLFW_PRESS) {
            keysToggle[key] = !keysToggle[key];
        }
    }
}

void InputManager::Char_Callback(GLFWwindow *window_, const unsigned int codepoint) {
    if (!InputActive) {
        if (!charBuffer.empty()) charBuffer.clear();
        return;
    }
    if (charBuffer.size() > 255) { return; }

    if (codepoint >= 32 && codepoint <= 126) {
        charBuffer.push_back(static_cast<char>(codepoint));
    }
}


void InputManager::Mouse_Callback(GLFWwindow* window_, const double xpos, double ypos) {
    MouseX = xpos;
    MouseY = ypos;
    if (!Camera.Mouse_Visible) {
        if (Camera.FirstMouse) {
            Camera.LastX = xpos;
            Camera.LastY = ypos;
            Camera.FirstMouse = false;
        }

        float xoffset = xpos - Camera.LastX;
        float yoffset = Camera.LastY - ypos;

        Camera.LastX = xpos;
        Camera.LastY = ypos;

        xoffset *= Camera.Sensitivity;
        yoffset *= Camera.Sensitivity;

        Camera.Yaw   += xoffset;
        Camera.Pitch += yoffset;

        Camera.Pitch = std::clamp(Camera.Pitch, -89.0f, 89.0f);
    }
}

void InputManager::Scroll_Callback(GLFWwindow* window_, double xoffset, double yoffset) {
    ScrollX = xoffset;
    ScrollY = yoffset;
}

void InputManager::Mouse_Key_Callback(GLFWwindow* window_, int button, int action, int mods) {
    if (button < 0 || button >= 8) return;

    if (action == GLFW_PRESS && !MouseState[button]) {
        MouseToggle[button] = !MouseToggle[button];
    }

    MouseState[button] = (action == GLFW_PRESS);
}

InputManager In;