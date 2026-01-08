#include <glad/glad.h>
#include "Gui.hpp"
#include "Common/Gui_Types.hpp"
#include <Utils/InputManager.hpp>

using namespace gui;

void Gui::HotBar() {
    constexpr float slotSize = 19.0f;
    constexpr float slotSpacing = 2.0f + slotSize;

    auto Size = glm::vec2(190.0f, 22.0f);
    DrawRectangle({Anch::BottomCenter, Size, {0.0f, -2.0f}}, {rgb(0x303030), Texture_Id::None});

    if (InputManager::ScrollY != 0) {
        Camera.HotBarSlot = wrap(Camera.HotBarSlot - InputManager::ScrollY, 9);
        Camera.ItemHeld = (Camera.HotBarSlot % 6) + 1;
        InputManager::ScrollY = 0;
    }

    Size = glm::vec2(slotSize);
    if (!Camera.Mode) {
        for (int i = 0; i < 9; i++) {
            DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.5f}}, {{i, 0,0,0}, Texture_Id::Block});
        }
        // Selected Slot
        const glm::vec4 color = rgb(0xfff072);
        Size = glm::vec2(23, 2);
        DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -1.5f}}, {color, Texture_Id::None});
        DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(Camera.HotBarSlot*slotSpacing), -22.5f}}, {color, Texture_Id::None});
        Size = glm::vec2(2, 19);
        DrawRectangle({Anch::BottomCenter, Size, {-94.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}}, {color, Texture_Id::None});
        DrawRectangle({Anch::BottomCenter, Size, {-73.5f+(Camera.HotBarSlot*slotSpacing), -3.5f}}, {color, Texture_Id::None});
    } else {
        for (int i = 0; i < 9; i++) {
            DrawRectangle({Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.0f}}, {rgb(0x404040), Texture_Id::None});
        }
    }
}

void Gui::Statistics() {
    constexpr float HalfHotBar = 188.0f/2;
    
    // Food
    auto StatSize = glm::vec2((HalfHotBar-10)+2, 10);
    DrawRectangle({Anch::BottomCenter, StatSize, {(-HalfHotBar + StatSize.x/2)-1, -25.0f}}, {rgb(0x404040), Texture_Id::None});
    StatSize = glm::vec2(HalfHotBar-10, 8);
    DrawProgressBar({Anch::BottomCenter, StatSize, {-HalfHotBar + StatSize.x/2, -26.0f}}, {1, rgb(0xff8c00), Texture_Id::None});

    // Water
    StatSize = glm::vec2((-HalfHotBar+10)-2, 10);
    DrawRectangle({Anch::BottomCenter, StatSize, {(HalfHotBar + StatSize.x/2)+1, -25.0f}}, {rgb(0x404040), Texture_Id::None});
    StatSize = glm::vec2(-HalfHotBar+10, 8);
    DrawProgressBar({Anch::BottomCenter, StatSize, {HalfHotBar + StatSize.x/2, -26.0f}}, {1, rgb(0x00f7ff), Texture_Id::None});
}

void Gui::Health() {
    constexpr auto Size = glm::vec2(100, 150);

    DrawRectangle({Anch::BottomLeft, Size, {1.0f, -1.0f}}, {rgb(0x404040), Texture_Id::None});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, -1.0f}}), {.text = R"( !"#$%&'()*+,.-/0123456789:;<=>?@{}~)"});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, 10.0f}}), {.text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, 20.0f}}), {.text = "abcdefghijklmnopqrstuvwxyz"});
}

void Gui::Crosschair() {
    const glm::vec4 color = rgb(0xffffff);

    auto Size = glm::vec2(0.5f, 5.0f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {color, Texture_Id::None});

    Size = glm::vec2(5.0f, 0.5f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {color, Texture_Id::None});
}

void Gui::Menu() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};
    
    // Resume
    Layout layout = {Anch::Center, {150, 20}, {0.0f, -25.0f}};
    Label label = {"Resume", .anchor = Anch::Center};
    if (Button(layout, Big, label)) {
        InputManager::Key_Callback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
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
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};

    Layout layout = {Anch::Center, {70, 20}, {-(70+10)/2, -25.0f}};
    Label label = {"Render +", .anchor = Anch::Center};

    if (Button(layout, Small, label)) {
        renderd += 1;
        if (Camera.RenderDistance+renderd > 64) renderd -= 1;
    }

    layout.Offset.y = 0;
    if (game.V_Sync == 1) {
        label.text = "V-sync On";
    } else {
        label.text = "V-Sync Off";
    }
    if (Button(layout, Small, label)) {
        if (game.V_Sync == 1) {
            game.V_Sync = 0;
        } else {
            game.V_Sync = 1;
        }
    }

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
        glfwSwapInterval(game.V_Sync);
    }

    static TextCache rd;
    UpdateText(rd, Camera.RenderDistance+renderd, "Render Distance: %d");
    layout.Offset = {0, -50};
    label.text = rd.text;
    Button(layout, Big, label);
}

void Gui::DebugScreen() {
    Layout layout = {Anch::TopLeft, {60, 60}, {1,1}};

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

    static ProgressStyle style = {.TextureId = Texture_Id::None};
    static Label label_ram = {.Style = {.Scale = 0.5}, .anchor = Anch::Center};
    static Label label_tris = {.Style = {.Scale = 0.5}, .anchor = Anch::Center};

    static size_t LastRam;
    const float ramUsedRatio = static_cast<float>(PerfS.ramUsed) / static_cast<float>(game.Max_Ram * 1024 * 1024);
    style.Progress = ramUsedRatio;
    if (LastRam != PerfS.ramUsed) {
        style.BgColor = Gradient(ramUsedRatio, rgb(0x00ff00), rgb(0xffff00), rgb(0xff0000));
        label_ram.text = Format("%s/%s", Fun::FormatSize(PerfS.ramUsed).c_str(),Fun::FormatSize(game.Max_Ram * 1024 * 1024).c_str());
        LastRam = PerfS.ramUsed;
    }
    layout.Offset.y = 37;
    layout.Size = {53, 10};
    DrawProgressBar(layout, style, &label_ram);

    static uint64_t LastTris;
    const float TrisVisibleRatio = static_cast<float>(PerfS.Triangles) / static_cast<float>(PerfS.Total_Triangles);
    style.Progress = TrisVisibleRatio;
    if (LastTris != PerfS.Triangles) {
        style.BgColor = Gradient(TrisVisibleRatio, rgb(0x00ff00), rgb(0xffff00), rgb(0xff0000));
        label_tris.text = Format("%s/%s", Fun::FormatNumber(PerfS.Triangles).c_str(), Fun::FormatNumber(PerfS.Total_Triangles).c_str());
        LastTris = PerfS.Triangles;
    }
    layout.Offset.y = 47;
    DrawProgressBar(layout, style, &label_tris);
}

void Gui::Chat() {
    Layout layout = {Anch::LeftCenter, {125, 100}, {1,50}};
    // Background
    DrawRectangle(layout, {{rgb(0x404040)}});
    layout.Size = {125, 10};
    layout.Offset.y = 95;
    DrawRectangle(layout, {{rgb(0x505050)}});
    Label label = {.Style = {.Scale = 0.5}, .anchor = Anch::LeftCenter};
    int y = Anchor(layout).y - 99;
    for (const auto& msg : chat) {
        label.text = msg;
        Text({3, y}, label);
        y += 10;
    }
    if (InputManager::keysState[GLFW_KEY_ENTER] && net.client.client) {
        const std::string msg = "Hello";
        Packet pkt = {.h = {PacketType::CHAT, static_cast<uint16_t>(msg.size())}};
        pkt.data.resize(msg.size());
        pkt.data.assign(msg.begin(), msg.end());
        memcpy(pkt.data.data(), msg.c_str(), msg.size());
        net.Client_Send(pkt);
        net.client.Send();
        InputManager::keysState[GLFW_KEY_ENTER] = false;
    }
    Packet p;
    if (net.Server_Read(p)) {
        if (p.h.type == PacketType::CHAT) {
            net.server.BroadcastPackets(p, nullptr);
        }
    }
    if (net.Client_Read(p)) {
        if (p.h.type == PacketType::CHAT) {
            chat.emplace_back(p.data.begin(), p.data.end());
        }
    }
}

void Gui::Generate() {
    if (!InputManager::keysToggle[GLFW_KEY_F3]) DebugScreen();

    HotBar();

    if (!Camera.Mode) Statistics();

    Crosschair();

    if (InputManager::keysState[GLFW_KEY_TAB] && !InputManager::keysToggle[GLFW_KEY_ESCAPE]) { Health(); }
    if (InputManager::keysToggle[GLFW_KEY_T] && !InputManager::keysState[GLFW_KEY_TAB] && !InputManager::keysToggle[GLFW_KEY_ESCAPE])
        { Chat(); Camera.Can_Move = false; }

    if (InputManager::keysToggle[GLFW_KEY_ESCAPE]) {
        if (InputManager::keysToggle[GLFW_KEY_T]) {InputManager::keysToggle[GLFW_KEY_T] = false; }
        Camera.Can_Move = false;
        if (game.MenuId == 0) { Menu(); }
        else if (game.MenuId == 1) { Settings(); }
        else if (game.MenuId == 2) { Multiplayer(); }
        else if (game.MenuId == 3) { MultiplayerJoin(); }
        else if (game.MenuId == 4) { MultiplayerHost(); }
    } else if (!InputManager::keysToggle[GLFW_KEY_T]) { Camera.Can_Move = true; }
}
