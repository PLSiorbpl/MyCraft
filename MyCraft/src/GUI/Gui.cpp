#include <glad/glad.h>
#include "Gui.hpp"
#include "Common/Gui_Types.hpp"
#include "Render/Camera.hpp"
#include <Utils/InputManager.hpp>
#include "World/Chunk.hpp"

using namespace gui;

void Gui::HotBar() {
    constexpr glm::vec2 Slot_Size = {24, 24};
    constexpr glm::vec2 Item_Size = {16, 16};
    constexpr glm::vec2 Hotbar_size = {(Slot_Size.x + 1) * 9 + 1, 24.0f};
    constexpr Layout Hotbar_layout = {Anch::BottomCenter, Hotbar_size, {0.0f, -1.0f}};
    Layout Slot_layout = {Anch::LeftCenter, Slot_Size, {0, 0}, &Hotbar_layout};
    const Layout Item_layout = {Anch::Center, Item_Size, {0, 0}, &Slot_layout};

    //DrawRectangle(Hotbar_layout, {rgba(0x303030D9), Texture_Id::None});

    if (InputManager::ScrollY != 0) {
        if (gui_state == Gui_State::None) {
            Camera.HotBarSlot = wrap(Camera.HotBarSlot - InputManager::ScrollY, 9);
            Camera.ItemHeld = Camera.Hotbar_slots[Camera.HotBarSlot];
        }
        InputManager::ScrollY = 0;
    }

    if (true) { // !Camera.Mode
        for (int i = 0; i < 9; i++) {
            if (i != Camera.HotBarSlot)
                DrawRectangle(Slot_layout, {{222, 0, 244, 21}, Texture_Id::Gui});
            else {
                DrawRectangle(Slot_layout, {{222, 23, 244, 44}, Texture_Id::Gui});
            }
            DrawRectangle(Item_layout, {{Camera.Hotbar_slots[i], 0,0,0}, Texture_Id::Block});
            Slot_layout.Move_X(1);
        }
    } else {
    }
}

void Gui::Inventory() {
    ID = 0;
    static int item_selected = -1;
    if (Camera.Mode || true) {
        constexpr glm::vec2 Inventory_Size = {371, 223};
        constexpr glm::vec2 Slot_Size = {36, 36};
        constexpr glm::vec2 Item_Size = {24, 24};
        constexpr Layout Inventory_layout = {.Anchor = Anch::Center, .Size = Inventory_Size, .Offset = {0, -15}};
        Layout Slot_layout = {.Anchor = Anch::TopLeft, .Size = Slot_Size, .Offset = {1, 1}, .Parent = &Inventory_layout};
        const Layout Item_layout = {.Anchor = Anch::Center, .Size = Item_Size, .Offset = {0, 0}, .Parent = &Slot_layout};

        DrawRectangle(Inventory_layout, {.BgColor = rgba(0x202020), .TextureId = Texture_Id::None});

        for (int y = 0; y < 6; y++) {
            for (int x = 0; x < 10; x++) {
                if (Button(Slot_layout, {.BgColor = {222, 46, 244, 67}, .HoverColor = {222, 23, 244, 44}, .TextureId = Texture_Id::Gui}, {})) {
                    item_selected = x+1+(y*10);
                }
                DrawRectangle(Item_layout, {.BgColor = {x+1+(y*10),0,0,0}, .TextureId = Texture_Id::Block});
                Slot_layout.Move_X(1);
            }
            Slot_layout.Move_Y(1);
            Slot_layout.Offset.x = 1;
        }

        if (item_selected != -1 && item_selected < static_cast<int>(block_type::_count)) {
            DrawRectangle({.Size = Item_Size, .Offset = game_settings.Mouse}, {.BgColor = {item_selected,0,0,0}, .TextureId = Texture_Id::Block});
        }

        constexpr glm::vec2 Hotbar_Slot_Size = {24, 24};
        constexpr glm::vec2 Hotbar_size = {(Hotbar_Slot_Size.x + 1) * 9 + 1, 24.0f};
        constexpr Layout Hotbar_layout = {Anch::BottomCenter, Hotbar_size, {0.0f, -1.0f}};
        Layout Hotbar_Slot_layout = {Anch::LeftCenter, Hotbar_Slot_Size, {0, 0}, &Hotbar_layout};

        for (int i = 0; i < 9; i++) {
            if (Button(Hotbar_Slot_layout, {}, {})) {
                if (item_selected != -1 && item_selected < static_cast<int>(block_type::_count)) {
                    if (InputManager::keysState[GLFW_KEY_LEFT_SHIFT]) { // Erase
                        Camera.Hotbar_slots[i] = 0;
                        item_selected = 0;
                    } else { // Replace
                        int temp = Camera.Hotbar_slots[i];
                        Camera.Hotbar_slots[i] = item_selected;
                        item_selected = temp;
                    }
                } else { // Get
                    item_selected = Camera.Hotbar_slots[i];
                    Camera.Hotbar_slots[i] = 0;
                }
                Camera.ItemHeld = Camera.Hotbar_slots[Camera.HotBarSlot];
                break;
            }
            Hotbar_Slot_layout.Move_X(1);
        }
    }
}

void Gui::Food_bar() {
    constexpr float HalfHotBar = 188.0f/2;
    
    // Food
    auto StatSize = glm::vec2((HalfHotBar-10)+2, 10);
    DrawRectangle({Anch::BottomCenter, StatSize, {(-HalfHotBar + StatSize.x/2)-1, -25.0f}}, {rgba(0x404040), Texture_Id::None});
    StatSize = glm::vec2(HalfHotBar-10, 8);
    ProgressBar({Anch::BottomCenter, StatSize, {-HalfHotBar + StatSize.x/2, -26.0f}}, {1, rgba(0xff8c00), Texture_Id::None});

    // Water
    StatSize = glm::vec2((-HalfHotBar+10)-2, 10);
    DrawRectangle({Anch::BottomCenter, StatSize, {(HalfHotBar + StatSize.x/2)+1, -25.0f}}, {rgba(0x404040), Texture_Id::None});
    StatSize = glm::vec2(-HalfHotBar+10, 8);
    ProgressBar({Anch::BottomCenter, StatSize, {HalfHotBar + StatSize.x/2, -26.0f}}, {1, rgba(0x00f7ff), Texture_Id::None});
}

void Gui::Health() {
    constexpr auto Size = glm::vec2(100, 150);

    DrawRectangle({Anch::BottomLeft, Size, {1.0f, -1.0f}}, {rgba(0x404040), Texture_Id::None});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, -1.0f}}), {.text = R"( !"#$%&'()*+,.-/0123456789:;<=>?@{}~)"});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, 10.0f}}), {.text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"});
    Text(Anchor({Anch::BottomLeft, Size, {1.0f, 20.0f}}), {.text = "abcdefghijklmnopqrstuvwxyz"});
}

void Gui::Crosschair() {
    const glm::vec4 color = rgba(0xffffff);

    auto Size = glm::vec2(0.5f, 5.0f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {color, Texture_Id::None});

    Size = glm::vec2(5.0f, 0.5f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {color, Texture_Id::None});
}

void Gui::Menu() {
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};
    static Animation_State<glm::vec2> Resume_State;
    static Animation_State<glm::vec2> Exit_State;
    static Animation_State<glm::vec2> Settings_State;
    static Animation_State<glm::vec2> Multiplayer_State;
    
    // Resume
    Layout layout = {Anch::Center, {150, 20}, {0.0f, -25.0f}};
    Label label = {.text = "Resume", .anchor = Anch::Center};
    if (Button(layout, Big, label, &Resume_State)) {
        Resume_State.state = State::Idle;
        InputManager::Key_Callback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
    }

    // Exit
    layout.Offset = {0.0f, 25.0f};
    label.text = "Exit";
    if (Button(layout, Big, label, &Exit_State)) {
        Exit_State.state = State::Idle;
        glfwSetWindowShouldClose(window, true);
    }
    // Settings
    layout.Size = {70, 20};
    layout.Offset = {-(layout.Size.x+10)/2, 0.0f};
    label.text = "Settings";
    if (Button(layout, Small, label, &Settings_State)) {
        Settings_State.state = State::Idle;
        game.MenuId = 1;
    }

    // Multiplayer
    layout.Offset = {(layout.Size.x+10)/2, 0.0f};
    label.text = "Multiplayer";
    if (Button(layout, Small, label, &Multiplayer_State)) {
        Multiplayer_State.state = State::Idle;
        Multiplayer();
        game.MenuId = 2;
    }
}

void Gui::Settings() {
    static int renderd = 0;
    ID = 0;
    static ButtonStyle Big = {{0,0,150,20}, {0,21,150,41}, Texture_Id::Gui};
    static ButtonStyle Small = {{151,0,221,20}, {151,21,221,41}, Texture_Id::Gui};
    static Animation_State<glm::vec2> Render1_State;
    static Animation_State<glm::vec2> Render2_State;
    static Animation_State<glm::vec2> Save_State;
    static Animation_State<glm::vec2> VSync_State;

    Layout layout = {Anch::Center, {70, 20}, {-(70+10)/2, -25.0f}};
    Label label = {.text = "Render +", .anchor = Anch::Center};
    if (Button(layout, Small, label, &Render1_State)) {
        Render1_State.state = State::Idle;
        renderd += 1;
        if (Camera.RenderDistance+renderd > 96) renderd -= 1;
    }

    layout.Offset.y = 0;
    if (game.V_Sync == 1) {
        label.text = "V-sync On";
    } else {
        label.text = "V-Sync Off";
    }
    if (Button(layout, Small, label, &VSync_State)) {
        VSync_State.state = State::Idle;
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
    if (Button(layout, Small, label, &Render2_State)) {
        Render2_State.state = State::Idle;
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
    if (Button(layout, Big, label, &Save_State)) {
        Save_State.state = State::Idle;
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
    /*
     FPS
     Frame time
     Mesh time
     Render time
     Gui time
     Tick time
     RAM
     Triangle
     Coordinates
     Looking direction
     Block held
     Looking at
     */

    Layout layout = {Anch::TopLeft, {80, (10*6)+(2*10)}, {1,1}};

    DrawRectangle(layout, {{rgba(0x101010)}});
    layout.Size = {0, 5};
    layout.Offset = {2, 1};

    static TextCache fps;
    UpdateText(fps, game.FPS, "FPS: %d");
    Text(Anchor(layout), {.text = fps.text, .Style = {.Scale = 0.5}});

    layout.Move_Y();
    Text(Anchor(layout), {.text = "Cpu Times:", .Style = {.Scale = 0.5}});

    static TextCache FrameTime;
    UpdateText(FrameTime, PerfS.EntireTime, "Frame Time: %.3fms");
    layout.Move_Y();
    Text(Anchor(layout), {.text = FrameTime.text, .Style = {.Scale = 0.5}});

    static TextCache MeshTime;
    UpdateText(MeshTime, PerfS.mesh, "Mesh Time: %.3fms");
    layout.Move_Y();
    Text(Anchor(layout), {.text = MeshTime.text, .Style = {.Scale = 0.5}});

    static TextCache RenderTime;
    UpdateText(RenderTime, PerfS.render, "Render Time: %.3fms");
    layout.Move_Y();
    Text(Anchor(layout), {.text = RenderTime.text, .Style = {.Scale = 0.5}});

    static TextCache GuiTime;
    UpdateText(GuiTime, PerfS.gui, "Gui Time: %.3fms");
    layout.Move_Y();
    Text(Anchor(layout), {.text = GuiTime.text, .Style = {.Scale = 0.5}});

    static TextCache TickTime;
    UpdateText(TickTime, PerfS.tick, "Tick Time: %.3fms");
    layout.Move_Y();
    Text(Anchor(layout), {.text = TickTime.text, .Style = {.Scale = 0.5}});

    static ProgressStyle ram_style = {.TextureId = Texture_Id::None};
    static ProgressStyle tri_style = {.TextureId = Texture_Id::None};
    static Label label_ram = {.Style = {.Scale = 0.5}, .anchor = Anch::Center};
    static Label label_tris = {.Style = {.Scale = 0.5}, .anchor = Anch::Center};

    static size_t LastRam;
    const float ramUsedRatio = static_cast<float>(PerfS.ramUsed) / static_cast<float>(game.Max_Ram * 1024 * 1024);
    ram_style.Progress = ramUsedRatio;
    if (LastRam != PerfS.ramUsed) {
        ram_style.BgColor = Gradient(ramUsedRatio, rgba(0x00ff00), rgba(0xffff00), rgba(0xff0000));
        label_ram.text = Format("%s/%s", Fun::FormatSize(PerfS.ramUsed).c_str(),Fun::FormatSize(game.Max_Ram * 1024 * 1024).c_str());
        LastRam = PerfS.ramUsed;
    }
    layout.Move_Y();
    layout.Size = {53, 10};
    ProgressBar(layout, ram_style, &label_ram);

    static uint64_t LastTris;
    const float TrisVisibleRatio = static_cast<float>(PerfS.Triangles) / static_cast<float>(PerfS.Total_Triangles);
    tri_style.Progress = TrisVisibleRatio;
    if (LastTris != PerfS.Triangles) {
        tri_style.BgColor = Gradient(TrisVisibleRatio, rgba(0x00ff00), rgba(0xffff00), rgba(0xff0000));
        label_tris.text = Format("%s/%s", Fun::FormatNumber(PerfS.Triangles).c_str(), Fun::FormatNumber(PerfS.Total_Triangles).c_str());
        LastTris = PerfS.Triangles;
    }
    layout.Move_Y();
    ProgressBar(layout, tri_style, &label_tris);

    layout.Move_Y();
    layout.Size = {0, 5};
    Text(Anchor(layout), {.text = Format("x: %.1f y: %.1f z: %.1f", Camera.Position.x, Camera.Position.y, Camera.Position.z), .Style = {.Scale = 0.5}});

    layout.Move_Y();
    Text(Anchor(layout), {.text = Format("Looking at: %s", Direction_to_String(Camera.direction).c_str()), .Style = {.Scale = 0.5}});

    if (block_cache[Camera.ItemHeld]) {
        layout.Move_Y();
        Text(Anchor(layout), {.text = Format("Block: %s", block_cache[Camera.ItemHeld]->get_name().c_str()), .Style = {.Scale = 0.5}});
    }

    if (Camera.looking_at) {
        layout.Move_Y();
        Text(Anchor(layout), {.text = Format("Looking at: %s", Camera.looking_at->get_name().c_str()), .Style = {.Scale = 0.5}});
    }
}

void Gui::Chat() {
    Layout layout = {Anch::LeftCenter, {125, 100}, {1,50}};
    // Background
    DrawRectangle(layout, {{rgba(0x404040)}});
    layout.Size = {125, 10};
    layout.Offset.y = 95;
    DrawRectangle(layout, {{rgba(0x505050)}});
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

void Gui::Update() {
    if (gui_state == Gui_State::Menu) {
        if (!InputManager::keysToggle[GLFW_KEY_ESCAPE]) {
            gui_state = Gui_State::None;
            InputManager::Set_Mouse_Visiblility(false);
            Camera.Can_Move = true;
            InputManager::InputActive = false;
            game.MenuId = 0;
            InputManager::keysToggle[GLFW_KEY_T] = false;
            InputManager::keysToggle[GLFW_KEY_E] = false;
        }
        return;
    }

    if (InputManager::keysToggle[GLFW_KEY_ESCAPE]) {
        gui_state = Gui_State::Menu;
        InputManager::Set_Mouse_Visiblility(true);
        Camera.Can_Move = false;
        game.MenuId = 0;
        return;
    }

    if (InputManager::keysToggle[GLFW_KEY_T]) {
        gui_state = Gui_State::Chat;
        InputManager::Set_Mouse_Visiblility(true);
        Camera.Can_Move = false;
        return;
    }

    if (InputManager::keysState[GLFW_KEY_TAB]) {
        gui_state = Gui_State::Health;
        InputManager::Set_Mouse_Visiblility(false);
        Camera.Can_Move = true;
        InputManager::InputActive = false;
        return;
    }

    if (InputManager::keysToggle[GLFW_KEY_E]) {
        gui_state = Gui_State::Inventory;
        InputManager::Set_Mouse_Visiblility(true);
        Camera.Can_Move = false;
        InputManager::InputActive = false;
        return;
    }

    if (gui_state == Gui_State::Chat || gui_state == Gui_State::Health || gui_state == Gui_State::Inventory) {
        gui_state = Gui_State::None;
        InputManager::Set_Mouse_Visiblility(false);
        Camera.Can_Move = true;
        InputManager::InputActive = false;
    }
}

void Gui::Generate() {
    if (!InputManager::keysToggle[GLFW_KEY_F3])
        DebugScreen();

    HotBar();

    if (!Camera.Mode)
        Food_bar();

    Crosschair();

    switch (gui_state) {
        case Gui_State::None:
            break;
        case Gui_State::Inventory: Inventory();
            break;
        case Gui_State::Health: Health();
            break;
        case Gui_State::Chat: Chat();
            break;
        case Gui_State::Menu:
            switch (game.MenuId) {
                case 0: Menu(); break;
                case 1: Settings(); break;
                case 2: Multiplayer(); break;
                case 3: MultiplayerJoin(); break;
                case 4: MultiplayerHost(); break;
                default: break;
            }
            break;

        default: break;
    }
}
