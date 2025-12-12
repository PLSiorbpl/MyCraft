#pragma once
#include <GLFW/glfw3.h>
#include <algorithm>

#include "Globals.hpp"

class InputManager {
public:
    static bool keysToggle[512];
    static bool keysState[512];
    static bool MouseState[8];
    static bool MouseToggle[8];
    static float MouseX;
    static float MouseY;
    static int ScrollX;
    static int ScrollY;
    static void Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void Mouse_Callback(GLFWwindow* window, double xpos, double ypos);
    static void Mouse_Key_Callback(GLFWwindow* window, int button, int action, int mods);
    static void Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset);
};

extern InputManager In;