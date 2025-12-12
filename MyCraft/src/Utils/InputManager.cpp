#include "InputManager.hpp"

bool InputManager::keysToggle[512] = {false};
bool InputManager::keysState[512] = {false};
bool InputManager::MouseState[8] = {false};
bool InputManager::MouseToggle[8] = {false};
float InputManager::MouseX = 0.0f;
float InputManager::MouseY = 0.0f;
int InputManager::ScrollX = 0;
int InputManager::ScrollY = 0;

void InputManager::Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        keysState[key] = (action != GLFW_RELEASE);

        if (action == GLFW_PRESS) {
            keysToggle[key] = !keysToggle[key];

            if (key == GLFW_KEY_ESCAPE) {
                glfwSetInputMode(window, GLFW_CURSOR, keysToggle[key] ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                Camera.Mouse_Visible = keysToggle[key];
                glfwSetCursorPos(window, game_settings.width/2, game_settings.height/2);
                MouseX = game_settings.width/2; MouseY = game_settings.height/2;
                Camera.LastX = game_settings.width/2; Camera.LastY = game_settings.height/2;
            }
        }
    }
}

void InputManager::Mouse_Callback(GLFWwindow* window, double xpos, double ypos) {
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

void InputManager::Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset) {
    ScrollX = xoffset;
    ScrollY = yoffset;
}

void InputManager::Mouse_Key_Callback(GLFWwindow* window, int button, int action, int mods) {
    if (button < 0 || button >= 8) return;

    if (action == GLFW_PRESS && !MouseState[button]) {
        MouseToggle[button] = !MouseToggle[button];
    }

    MouseState[button] = (action == GLFW_PRESS);
}

InputManager In;