#include <glad/glad.h>
#include "Gui.hpp"

void Gui::HotBar() {
    uint32_t Flags = 0;

    constexpr float slotSize = 19.0f;
    constexpr float slotSpacing = 2.0f + slotSize;

    glm::vec2 Size = glm::vec2(190.0f, 22.0f);
    DrawRectangle(Anchor(Anch::BottomCenter, Size, {0.0f, -2.0f}), Size, rgb(0x303030), Flags);

    if (In.ScrollY != 0) {
        Camera.HotBarSlot = wrap(Camera.HotBarSlot - In.ScrollY, 9);
        Camera.ItemHeld = (Camera.HotBarSlot % 6) + 1;
        In.ScrollY = 0;
    }

    Size = glm::vec2(slotSize);
    if (!Camera.Mode) {
        for (int i = 0; i < 9; i++) {
            DrawRectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.5f}), Size, Texture(texture::Block, i, 0, Flags), Flags);
        }
        // Selected Slot
        glm::vec4 color = rgb(0xfff072);
        Size = glm::vec2(23, 2);
        DrawRectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -1.5f}), Size, Color(color, Flags), Flags);
        DrawRectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -22.5f}), Size, Color(color, Flags), Flags);
        Size = glm::vec2(2, 19);
        DrawRectangle(Anchor(Anch::BottomCenter, Size, {-94.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}), Size, Color(color, Flags), Flags);
        DrawRectangle(Anchor(Anch::BottomCenter, Size, {-73.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}), Size, Color(color, Flags), Flags);
    } else {
        for (int i = 0; i < 9; i++) {
            DrawRectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.0f}), Size, Color(rgb(0x404040), Flags), Flags);
        }
    }
}

void Gui::Statistics() {
    uint32_t Flags = 0;
    glm::vec4 UV = glm::vec4(0);

    float HalfHotBar = 188.0f/2;
    
    // Food
    glm::vec2 StatSize = glm::vec2((HalfHotBar-10)+2, 10);
    DrawRectangle(Anchor(Anch::BottomCenter, StatSize, {(-HalfHotBar + StatSize.x/2)-1, -25.0f}), StatSize, Color(rgb(0x404040), Flags), Flags);
    StatSize = glm::vec2(HalfHotBar-10, 8);
    DrawProgressBar(1, Anchor(Anch::BottomCenter, StatSize, {-HalfHotBar + StatSize.x/2, -26.0f}), StatSize, Color(rgb(0xff8c00), Flags), Flags);

    // Water
    StatSize = glm::vec2((-HalfHotBar+10)-2, 10);
    DrawRectangle(Anchor(Anch::BottomCenter, StatSize, {(HalfHotBar + StatSize.x/2)+1, -25.0f}), StatSize, Color(rgb(0x404040), Flags), Flags);
    StatSize = glm::vec2(-HalfHotBar+10, 8);
    DrawProgressBar(1, Anchor(Anch::BottomCenter, StatSize, {HalfHotBar + StatSize.x/2, -26.0f}), StatSize, Color(rgb(0x00f7ff), Flags), Flags);
}

void Gui::Health() {
    uint32_t Flags = 0;
    glm::vec2 Size = glm::vec2(100, 150);

    DrawRectangle(Anchor(Anch::BottomLeft, Size, {1.0f, -1.0f}), Size, Color(rgb(0x404040), Flags), Flags);
}

void Gui::Crosschair() {
    uint32_t Flags = 0;
    glm::vec4 color = rgb(0xffffff);

    glm::vec2 Size = glm::vec2(0.5f, 5.0f);
    DrawRectangle(Anchor(Anch::Center, Size), Size, Color(color, Flags), Flags);

    Size = glm::vec2(5.0f, 0.5f);
    DrawRectangle(Anchor(Anch::Center, Size), Size, Color(color, Flags), Flags);
}

void Gui::Menu() {
    uint32_t Flags = 0;
    glm::vec2 Size = glm::vec2(width, height);

    //Rectangle({0.0f, 0.0f}, Size, Color(rgb(0x404040), Flags), Flags);

    glm::vec2 Pos;
    glm::vec4 Unfocuse = Texture(texture::Gui, 0, 0, Flags, {0,0,150,20});
    glm::vec4 Hover = Texture(texture::Gui, 0, 0, Flags, {0,21,150,41});
    
    // Resume
    Size = glm::vec2(150, 20);
    Pos = Anchor(Anch::Center, Size, {0.0f, -25.0f});
    if (Button(Pos, Size, Flags, 1, Unfocuse, Hover)) {
        In.Key_Callback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
    }
    Text(Pos, "Resume", 1, Flags, {1,0}, {53, 4});

    // Exit
    Pos = Anchor(Anch::Center, Size, {0.0f, 25.0f});
    if (Button(Pos, Size, Flags, 1, Unfocuse, Hover)) {
        glfwSetWindowShouldClose(window, true);
    }
    Text(Pos, "Exit", 1, Flags, {1,0}, {65, 4});
    
    // Settings
    Unfocuse = Texture(texture::Gui, 0, 0, Flags, {151,0,221,20});
    Hover = Texture(texture::Gui, 0, 0, Flags, {151,21,221,41});
    Size = glm::vec2(70, 20);

    Pos = Anchor(Anch::Center, Size, {-(Size.x+10)/2, 0.0f});
    if (Button(Pos, Size, Flags, 1 , Unfocuse, Hover)) {
        game.MenuId = 1;
    }
    Text(Pos, "Options", 1, Flags, {1,0}, {14, 4});

    // idk
    Pos = Anchor(Anch::Center, Size, {(Size.x+10)/2, 0.0f});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);
    Text(Pos, "Something", 1, Flags, {1,0}, {11, 4});
}

void Gui::Settings() {
    uint32_t Flags = 0;
    glm::vec2 Size = glm::vec2(width, height);

    glm::vec2 Pos;
    glm::vec4 Unfocuse = Texture(texture::Gui, 0, 0, Flags, {151,0,221,20});
    glm::vec4 Hover = Texture(texture::Gui, 0, 0, Flags, {151,21,221,41});
    Size = glm::vec2(70, 20);

    Pos = Anchor(Anch::Center, Size, {-(Size.x+10)/2, 0});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);
    Pos = Anchor(Anch::Center, Size, {-(Size.x+10)/2, 25});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);
    Pos = Anchor(Anch::Center, Size, {-(Size.x+10)/2, 50});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);

    Pos = Anchor(Anch::Center, Size, {(Size.x+10)/2, 0});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);
    Pos = Anchor(Anch::Center, Size, {(Size.x+10)/2, 25});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);
    Pos = Anchor(Anch::Center, Size, {(Size.x+10)/2, 50});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);

    Unfocuse = Texture(texture::Gui, 0, 0, Flags, {0,0,150,20});
    Hover = Texture(texture::Gui, 0, 0, Flags, {0,21,150,41});
    Size = glm::vec2(150, 20);
    Pos = Anchor(Anch::Center, Size, {0, 75});
    Button(Pos, Size, Flags, 1, Unfocuse, Hover);
}

void Gui::Generate(const int width, const int heigh, const float Scale) {
    Clear(width, heigh, Scale);

    HotBar();
    if (!Camera.Mode) Statistics();
    Crosschair();
    if (In.keysState[GLFW_KEY_TAB] && !In.keysToggle[GLFW_KEY_ESCAPE]) { Health(); }
    if (In.keysToggle[GLFW_KEY_ESCAPE]) {
        if (game.MenuId == 0) { Menu(); Camera.Can_Move = false; }
        else if (game.MenuId == 1) { Settings(); Camera.Can_Move = false; }
    }
    else { Camera.Can_Move = true; }

    Send_Data();
}
