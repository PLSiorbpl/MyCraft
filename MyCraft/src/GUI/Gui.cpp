#include <glad/glad.h>
#include "Gui.hpp"

void Gui::HotBar() {
    uint32_t Flags = 0;

    constexpr float slotSize = 19.0f;
    constexpr float slotSpacing = 2.0f + slotSize;

    glm::vec2 Size = glm::vec2(190.0f, 22.0f);
    Rectangle(Anchor(Anch::BottomCenter, Size, {0.0f, -2.0f}), Size, rgb(0x303030), Flags);

    if (In.ScrollY != 0) {
        Camera.HotBarSlot = wrap(Camera.HotBarSlot - In.ScrollY, 9);
        Camera.ItemHeld = (Camera.HotBarSlot % 6) + 1;
        In.ScrollY = 0;
    }

    Size = glm::vec2(slotSize);
    if (!Camera.Mode) {
        for (int i = 0; i < 9; i++) {
            Rectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.5f}), Size, Texture(texture::Block, i, 0, Flags), Flags);
        }
        // Selected Slot
        glm::vec4 color = rgb(0xfff072);
        Size = glm::vec2(23, 2);
        Rectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -1.5f}), Size, Color(color, Flags), Flags);
        Rectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -22.5f}), Size, Color(color, Flags), Flags);
        Size = glm::vec2(2, 19);
        Rectangle(Anchor(Anch::BottomCenter, Size, {-94.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}), Size, Color(color, Flags), Flags);
        Rectangle(Anchor(Anch::BottomCenter, Size, {-73.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}), Size, Color(color, Flags), Flags);
    } else {
        for (int i = 0; i < 9; i++) {
            Rectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.0f}), Size, Color(rgb(0x404040), Flags), Flags);
        }
    }
}

void Gui::Statistics() {
    uint32_t Flags = 0;
    glm::vec4 UV = glm::vec4(0);

    float HalfHotBar = 188.0f/2;
    glm::vec2 StatSize = glm::vec2(8.5f, 8.5f);
    float StatSpacing = 0.5f + StatSize.x;
    
    // Food
    for (int i = 0; i < 10; i++) {
        Rectangle(Anchor(Anch::BottomCenter, StatSize, {-HalfHotBar + StatSize.x/2 + (i*StatSpacing), -26.0f}), StatSize, Color(rgb(0xff8c00), Flags), Flags);
    }

    // Water
    for (int i = 0; i < 10; i++) {
        Rectangle(Anchor(Anch::BottomCenter, StatSize, {0.0f + StatSize.x + (i*StatSpacing), -26.0f}), StatSize, Color(rgb(0x00f7ff), Flags), Flags);
    }
}

void Gui::Health() {
    uint32_t Flags = 0;
    glm::vec2 Size = glm::vec2(100, 150);

    Rectangle(Anchor(Anch::BottomLeft, Size, {1.0f, -1.0f}), Size, Color(rgb(0x404040), Flags), Flags);
}

void Gui::Crosschair() {
    uint32_t Flags = 0;
    glm::vec4 color = rgb(0xffffff);

    glm::vec2 Size = glm::vec2(0.5f, 5.0f);
    Rectangle(Anchor(Anch::Center, Size), Size, Color(color, Flags), Flags);

    Size = glm::vec2(5.0f, 0.5f);
    Rectangle(Anchor(Anch::Center, Size), Size, Color(color, Flags), Flags);
}

void Gui::Menu() {
    uint32_t Flags = 0;
    glm::vec2 Size = glm::vec2(width, height);

    //Rectangle({0.0f, 0.0f}, Size, Color(rgb(0x404040), Flags), Flags);

    glm::vec2 Pos;
    glm::vec4 Col = rgb(0x404040);
    Size = glm::vec2(150, 20);
    Rectangle(Anchor(Anch::Center, Size, {0.0f, -25.0f}), Size, Color(rgb(0x404040), Flags), Flags);

    Pos = Anchor(Anch::Center, Size, {0.0f, 25.0f});
    if (MouseInRect(Pos, Size)) {
        Col = rgb(0xffffff);
        if (In.MouseState[GLFW_MOUSE_BUTTON_1]) {
            glfwSetWindowShouldClose(window, true);
        }
    }
    Rectangle(Pos, Size, Color(Col, Flags), Flags);

    Size = glm::vec2(70, 20);
    Rectangle(Anchor(Anch::Center, Size, {-(Size.x+10)/2, 0.0f}), Size, Color(rgb(0x404040), Flags), Flags);
    Rectangle(Anchor(Anch::Center, Size, {(Size.x+10)/2, 0.0f}), Size, Color(rgb(0x404040), Flags), Flags);
}

void Gui::Generate(const int width, const int heigh, const float Scale) {
    Clear(width, heigh, Scale);

    HotBar();
    if (!Camera.Mode) Statistics();
    Crosschair();
    if (In.keysState[GLFW_KEY_TAB] && !In.keysToggle[GLFW_KEY_ESCAPE]) { Health(); }
    if (In.keysToggle[GLFW_KEY_ESCAPE]) { Menu(); Camera.Can_Move = false; }
    else { Camera.Can_Move = true; }

    Send_Data();
}
