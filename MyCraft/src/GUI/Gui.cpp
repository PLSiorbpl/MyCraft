#include <glad/glad.h>
#include "Gui.hpp"

void Gui::HotBar() {
    constexpr float slotSize = 19.0f;
    constexpr float slotSpacing = 2.0f + slotSize;

    glm::vec2 Size = glm::vec2(190.0f, 22.0f);
    DrawRectangle({Anch::BottomCenter, Size, {0.0f, -2.0f}}, {rgb(0x303030), texture::None});

    if (In.ScrollY != 0) {
        Camera.HotBarSlot = wrap(Camera.HotBarSlot - In.ScrollY, 9);
        Camera.ItemHeld = (Camera.HotBarSlot % 6) + 1;
        In.ScrollY = 0;
    }

    Size = glm::vec2(slotSize);
    if (!Camera.Mode) {
        for (int i = 0; i < 9; i++) {
            DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.5f}}, {{i, 0,0,0}, texture::Block});
        }
        // Selected Slot
        glm::vec4 color = rgb(0xfff072);
        Size = glm::vec2(23, 2);
        DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -1.5f}}, {color, texture::None});
        DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -22.5f}}, {color, texture::None});
        Size = glm::vec2(2, 19);
        DrawRectangle({Anch::BottomCenter, Size, {-94.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}}, {color, texture::None});
        DrawRectangle({Anch::BottomCenter, Size, {-73.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}}, {color, texture::None});
    } else {
        for (int i = 0; i < 9; i++) {
            DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.0f}}, {rgb(0x404040), texture::None});
        }
    }
}

void Gui::Statistics() {
    glm::vec4 UV = glm::vec4(0);

    float HalfHotBar = 188.0f/2;
    
    // Food
    glm::vec2 StatSize = glm::vec2((HalfHotBar-10)+2, 10);
    DrawRectangle({Anch::BottomCenter, StatSize, {(-HalfHotBar + StatSize.x/2)-1, -25.0f}}, {rgb(0x404040), texture::None});
    StatSize = glm::vec2(HalfHotBar-10, 8);
    DrawProgressBar({Anch::BottomCenter, StatSize, {-HalfHotBar + StatSize.x/2, -26.0f}}, {1, rgb(0xff8c00), texture::None});

    // Water
    StatSize = glm::vec2((-HalfHotBar+10)-2, 10);
    DrawRectangle({Anch::BottomCenter, StatSize, {(HalfHotBar + StatSize.x/2)+1, -25.0f}}, {rgb(0x404040), texture::None});
    StatSize = glm::vec2(-HalfHotBar+10, 8);
    DrawProgressBar({Anch::BottomCenter, StatSize, {HalfHotBar + StatSize.x/2, -26.0f}}, {1, rgb(0x00f7ff), texture::None});
}

void Gui::Health() {
    glm::vec2 Size = glm::vec2(100, 150);

    DrawRectangle({Anch::BottomLeft, Size, {1.0f, -1.0f}}, {rgb(0x404040), texture::None});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, -1.0f}}), {.text = R"( !"#$%&'()*+,.-/0123456789:;<=>?@{}~)"});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, 10.0f}}), {.text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, 20.0f}}), {.text = "abcdefghijklmnopqrstuvwxyz"});
}

void Gui::Crosschair() {
    uint32_t Flags = 0;

    glm::vec4 color = rgb(0xffffff);

    glm::vec2 Size = glm::vec2(0.5f, 5.0f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {color, texture::None});

    Size = glm::vec2(5.0f, 0.5f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {color, texture::None});
}

void Gui::Menu() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, texture::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, texture::Gui};
    
    // Resume
    Layout layout = {Anch::Center, {150, 20}, {0.0f, -25.0f}};
    Label label = {"Resume", .anchor = Anch::Center};
    if (Button(layout, Big, label)) {
        In.Key_Callback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
    }

    // Exit
    layout.Offset = {0.0f, 25.0f};
    label.text = "Exit";
    if (Button(layout, Big, label)) {
        glfwSetWindowShouldClose(window, true);
    }
    // Settings
    layout.Size = {70, 20};
    layout.Offset = {-(layout.Size.x+10)/2, 0.0f};
    label.text = "Settings";
    if (Button(layout, Small, label)) {
        game.MenuId = 1;
    }

    // idk
    layout.Offset = {(layout.Size.x+10)/2, 0.0f};
    label.text = "Multiplayer";
    if (Button(layout, Small, label)) {
        game.MenuId = 2;
    }
}

void Gui::Settings() {
    static int renderd = 0;
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, texture::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, texture::Gui};

    Layout layout = {Anch::Center, {70, 20}, {-(70+10)/2, -25.0f}};
    Label label = {"Render +", .anchor = Anch::Center};

    if (Button(layout, Small, label)) {
        renderd += 1;
        if (Camera.RenderDistance+renderd > 48) renderd -= 1;
    }

    layout.Offset.y = 0;
    label.text = "???";
    Button(layout, Small, label);

    layout.Offset.y = 25;
    label.text = "???";
    Button(layout, Small, label);


    layout.Offset = {(layout.Size.x+10)/2, -25};
    label.text = "Render -";
    if (Button(layout, Small, label)) {
        renderd -= 1;
        if (Camera.RenderDistance+renderd < 2) renderd += 1;
    }

    layout.Offset.y = 0;
    label.text = "???";
    Button(layout, Small, label);

    layout.Offset.y = 25;
    label.text = "???";
    Button(layout, Small, label);


    layout.Offset = {0, 50};
    layout.Size = {150, 20};
    label.text = "Save";
    if (Button(layout, Big, label)) {
        Camera.RenderDistance += renderd;
        renderd = 0;
        game.MenuId = 0;
        game.Last_Chunk = glm::ivec3(9999);
    }

    static TextCache rd;
    UpdateText(rd, Camera.RenderDistance+renderd, "Render Dist: %d");
    layout.Offset = {0, -50};
    label.text = rd.text;
    Button(layout, Big, label);
}

void Gui::Multiplayer() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, texture::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, texture::Gui};

    Layout layout = {Anch::Center, {150, 20}, {0, 50}};
    Label label = {.anchor = Anch::Center};
    
    label.text = "Back";
    if (Button(layout, Big, label)) {
        game.MenuId = 0;
    }

    layout.Offset.y = 25;
    label.text = "Last (wip)";
    if (Button(layout, Big, label)) {

    }

    layout.Offset.y = -25;
    label.text = "Host";
    if (Button(layout, Big, label)) {

    }

    layout.Offset.y = 0;
    layout.Size.x = 70;
    label.text = "Join";
    if (Button(layout, Small, label)) {
        game.MenuId = 3;
    }
}

void Gui::MultiplayerJoin() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, texture::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, texture::Gui};

    Layout layout = {Anch::Center, {70, 20}, {0, 25}};
    Label label = {.anchor = Anch::Center};

    label.text = "Join";
    if (Button(layout, Small, label)) {
        // lol scary stuff here
    }

    layout.Size = {150, 20};
    layout.Offset.y = 50;
    label.text = "Back";
    if (Button(layout, Big, label)) {
        game.MenuId = 2;
    }

    layout.Offset.y = 0;
    label.anchor = Anch::LeftCenter;
    label.Offset.x = 3;
    label.text = "Port:";
    Button(layout, Big, label);

    layout.Offset.y = -25;
    label.text = "Ip:";
    Button(layout, Big, label);
}

void Gui::Performance() {
    Layout layout = {Anch::TopLeft, {60, 50}, {1,1}};

    DrawRectangle(layout, {{rgb(0x101010)}});

    static TextCache fps;
    UpdateText(fps, game.FPS, "FPS: %d");
    layout.Offset = {2,1};
    Text(Anchor(layout), {.text = fps.text, .Style = {.Scale = 0.5}});

    layout.Offset.y = 6;
    Text(Anchor(layout), {.text = "Cpu Times:", .Style = {.Scale = 0.5}});
    static TextCache FrameTime;
    UpdateText(FrameTime, PerfS.EntireTime, "Frame Time: %.3fms");
    layout.Offset.y = 11;
    Text(Anchor(layout), {.text = FrameTime.text, .Style = {.Scale = 0.5}});

    static TextCache MeshTime;
    UpdateText(MeshTime, PerfS.mesh, "Mesh Time: %.3fms");
    layout.Offset.y = 16;
    Text(Anchor(layout), {.text = MeshTime.text, .Style = {.Scale = 0.5}});

    static TextCache RenderTime;
    UpdateText(RenderTime, PerfS.render, "Render Time: %.3fms");
    layout.Offset.y = 21;
    Text(Anchor(layout), {.text = RenderTime.text, .Style = {.Scale = 0.5}});

    static TextCache GuiTime;
    UpdateText(GuiTime, PerfS.gui, "Gui Time: %.3fms");
    layout.Offset.y = 26;
    Text(Anchor(layout), {.text = GuiTime.text, .Style = {.Scale = 0.5}});

    static TextCache TickTime;
    UpdateText(TickTime, PerfS.tick, "Tick Time: %.3fms");
    layout.Offset.y = 31;
    Text(Anchor(layout), {.text = TickTime.text, .Style = {.Scale = 0.5}});

    static size_t LastRam;
    const float ramUsedRatio = float(PerfS.ramUsed) / float(game.Max_Ram * 1024 * 1024);
    static ProgressStyle style = {.Progress = (ramUsedRatio), .TextureId = texture::None};
    static Label label = {.Style = {.Scale = 0.5}, .anchor = Anch::Center};
    if (LastRam != PerfS.ramUsed) {
        style.BgColor = Gradient(ramUsedRatio, rgb(0x00ff00), rgb(0xffff00), rgb(0xff0000));
        label.text = Format("%s/%s", fun.FormatSize(PerfS.ramUsed).c_str(),fun.FormatSize(game.Max_Ram * 1024 * 1024).c_str());
        LastRam = PerfS.ramUsed;
    }
    layout.Offset.y = 37;
    layout.Size = {53, 10};
    DrawProgressBar(layout, style, &label);
}

void Gui::Generate(const int width, const int heigh, const float Scale) {
    Clear(width, heigh, Scale);

    if (!In.keysToggle[GLFW_KEY_F3]) Performance();

    HotBar();

    if (!Camera.Mode) Statistics();

    Crosschair();

    if (In.keysState[GLFW_KEY_TAB] && !In.keysToggle[GLFW_KEY_ESCAPE]) { Health(); }

    if (In.keysToggle[GLFW_KEY_ESCAPE]) {
        Camera.Can_Move = false;
        if (game.MenuId == 0) { Menu(); }
        else if (game.MenuId == 1) { Settings(); }
        else if (game.MenuId == 2) { Multiplayer(); }
        else if (game.MenuId == 3) { MultiplayerJoin(); }
    } else { Camera.Can_Move = true; }

    Send_Data();
}
