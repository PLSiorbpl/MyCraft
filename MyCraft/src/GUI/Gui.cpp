#include <glad/glad.h>
#include "Gui.hpp"
#include "Common/Gui_Types.hpp"
#include "Render/Camera.hpp"
#include <Utils/InputManager.hpp>
#include "World/Chunk.hpp"
#include <algorithm>
#include <format>

#include "Common/common.hpp"
#include "Common/Textures.hpp"
#include "World/Generation.hpp"
#include "World/Mesh/Mesh.hpp"

using namespace gui;

// Hotbar
constexpr glm::vec2 Slot_Size = {24, 24};
constexpr glm::vec2 Item_Size = {16, 16};
constexpr glm::vec2 Hotbar_size = {(Slot_Size.x + 1) * 9 + 1, 24.0f};
constexpr Layout Hotbar_layout = {Anch::BottomCenter, Hotbar_size, {0.0f, -1.0f}};
// Inventory
constexpr glm::vec2 Inventory_Size = {371, 223};
constexpr glm::vec2 Inventory_Slot_Size = {36, 36};
constexpr glm::vec2 Inventory_Item_Size = {24, 24};
constexpr Layout Inventory_layout = {.Anchor = Anch::Center, .Size = Inventory_Size, .Offset = {0, -15}};

void Gui::HotBar() {
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
                DrawRectangle(Slot_layout, {Slot_Round, Texture_Id::Gui});
            else {
                DrawRectangle(Slot_layout, {Slot_Active, Texture_Id::Gui});
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
        Layout Inventory_Slot_layout = {.Anchor = Anch::TopLeft, .Size = Inventory_Slot_Size, .Offset = {1, 1}, .Parent = &Inventory_layout};
        const Layout Item_layout = {.Anchor = Anch::Center, .Size = Inventory_Item_Size, .Offset = {0, 0}, .Parent = &Inventory_Slot_layout};

        DrawRectangle(Inventory_layout, {.UV = rgba(0x202020), .TextureId = Texture_Id::None});

        for (int y = 0; y < 6; y++) {
            for (int x = 0; x < 10; x++) {
                if (Button(Inventory_Slot_layout, {.BgColor = Slot, .HoverColor = Slot_Active, .TextureId = Texture_Id::Gui}, {})) {
                    item_selected = x+1+(y*10);
                }
                DrawRectangle(Item_layout, {.UV = {x+1+(y*10),0,0,0}, .TextureId = Texture_Id::Block});
                Inventory_Slot_layout.Move_X(1);
            }
            Inventory_Slot_layout.Move_Y(1);
            Inventory_Slot_layout.Offset.x = 1;
        }

        if (item_selected != -1 && item_selected < static_cast<int>(block_type::_count)) {
            DrawRectangle({.Size = Item_Size, .Offset = game_settings.Mouse}, {.UV = {item_selected,0,0,0}, .TextureId = Texture_Id::Block});
        }

        Layout Slot_layout = {Anch::LeftCenter, Slot_Size, {0, 0}, &Hotbar_layout};

        for (int i = 0; i < 9; i++) {
            if (Button(Slot_layout, {}, {})) {
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
            Slot_layout.Move_X(1);
        }
    }
}

void Gui::Food_bar() {
    constexpr float HalfHotBar = Hotbar_size.x/2;
    constexpr auto StatSize = glm::vec2((HalfHotBar-10), 10);
    constexpr Layout Food_layout = {Anch::TopLeft, StatSize, {2, -StatSize.y - 2}, &Hotbar_layout};
    constexpr Layout Water_layout = {Anch::TopRight, StatSize, {-2, -StatSize.y - 2}, &Hotbar_layout};
    Layout Progress_layout = {Anch::LeftCenter, {StatSize.x-2, 8}, {1, 0}};

    // Food
    Progress_layout.Parent = &Food_layout;
    DrawRectangle(Food_layout, {.UV = rgba(0x404040)});
    ProgressBar(Progress_layout, {game.TimeOfDay, rgba(0xff8c00), Texture_Id::None, Widget_Direction::Right});

    // Water
    Progress_layout.Parent = &Water_layout;
    DrawRectangle(Water_layout, {.UV = rgba(0x404040)});
    ProgressBar(Progress_layout, {game.TimeOfDay, rgba(0x00f7ff), Texture_Id::None, Widget_Direction::Left});
}

void Gui::Health() {
    constexpr auto Size = glm::vec2(100, 150);
    constexpr Layout Health_layout = {.Anchor = Anch::BottomLeft, .Size = Size, .Offset = {1.0f, -1.0f}};
    Layout Text_layout = {.Anchor = Anch::TopLeft, .Size = {}, .Offset = {1, 1}, .Parent = &Health_layout};

    DrawRectangle(Health_layout, {rgba(0x404040), Texture_Id::None});
    Text(Anchor(Text_layout), {.text = R"( !"#$%&'()*+,.-/0123456789:;<=>?@{}~)"});
    Text_layout.Move_Y(10);
    Text(Anchor(Text_layout), {.text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"});
    Text_layout.Move_Y(10);
    Text(Anchor(Text_layout), {.text = "abcdefghijklmnopqrstuvwxyz"});
    Text_layout.Move_Y(10);
    Text(Anchor(Text_layout), {.text = "&aGreen &cRed #ff00ffPink #55ff55Alpha #ff1010&&## #ffff00literal &rreset #ffbad"});
}

void Gui::Crosschair() {
    const glm::vec4 color = rgba(0xffffffD9);

    auto Size = glm::vec2(0.5f, 5.0f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {.UV = color});

    Size = glm::vec2(5.0f, 0.5f);
    DrawRectangle({.Anchor = Anch::Center, .Size = Size}, {.UV = color});
}

void Gui::Menu() {
    ID = 0;
    static Animation_State<glm::vec2> Resume_State;
    static Animation_State<glm::vec2> Settings_State;
    static Animation_State<glm::vec2> Multiplayer_State;
    static Animation_State<glm::vec2> Exit_State;

    constexpr ButtonStyle Big = {.BgColor = Button_Big, .HoverColor = Button_Big_Active, .TextureId = Texture_Id::Gui};
    constexpr ButtonStyle Small = {.BgColor = Button_Small, .HoverColor = Button_Small_Active, .TextureId = Texture_Id::Gui};
    Layout Menu_layout = {.Anchor = Anch::Center, .Size = {150, 70}, .Offset = {}};
    Layout layout = {.Anchor = Anch::TopLeft, .Size = {150, 20}, .Offset = {}, .Parent = &Menu_layout};
    
    // Resume
    Label label = {.text = "Resume", .anchor = Anch::Center};
    layout.Center_animation(Resume_State, false);
    if (Button(layout, Big, label, &Resume_State)) {
        InputManager::Key_Callback(window, GLFW_KEY_ESCAPE, 0, GLFW_PRESS, 0);
    }
    layout.Center_animation(Resume_State, true);

    // Settings
    layout.Move_Y(5);
    label.text = "Settings";
    layout.Size = {70, 20};
    layout.Center_animation(Settings_State, false);
    if (Button(layout, Small, label, &Settings_State)) {
        game.MenuId = 1;
    }
    layout.Center_animation(Settings_State, true);

    // Multiplayer
    label.text = "Multiplayer";
    layout.Size = {70, 20}; layout.Move_X(10);
    layout.Center_animation(Multiplayer_State, false);
    if (Button(layout, Small, label, &Multiplayer_State)) {
        Multiplayer();
        game.MenuId = 2;
    }
    layout.Move_X(10, true); layout.Center_animation(Multiplayer_State, true);

    // Exit
    layout.Move_Y(5);
    label.text = "Exit";
    layout.Size = {150, 20};
    layout.Center_animation(Exit_State, false);
    if (Button(layout, Big, label, &Exit_State)) {
        glfwSetWindowShouldClose(window, true);
    }
    layout.Center_animation(Exit_State, true);
}

void Gui::Settings() {
    ID = 0;
    static int renderd = 0; static float y = 0;
    static ButtonStyle Big = {Button_Big, Button_Big_Active, Texture_Id::Gui};
    static ButtonStyle Small = {Button_Small, Button_Small_Active, Texture_Id::Gui};
    const Layout Menu_layout = {.Anchor = Anch::Center, .Size = {150, y}, .Offset = {}};
    Layout layout = {.Anchor = Anch::TopLeft, .Size = {70, 20}, .Offset = {}, .Parent = &Menu_layout};
    Layout layout_right = {.Anchor = Anch::TopRight, .Size = {70, 20}, .Offset = {}, .Parent = &Menu_layout};
    Label label = {.text = {}, .anchor = Anch::Center};
    static SliderStyle Big_slider = {0.5, Button_Big, Slider_H, Slider_H_Active, Texture_Id::Gui, 7};

    static Animation_State<glm::vec2> Save_State;
    static Animation_State<glm::vec2> VSync_State;
    y = 0;

    layout.Size = {150, 20};
    renderd = static_cast<int>(std::lerp(2, 256, Big_slider.Value));
    label.text = std::format("Render Distance {}", renderd);
    Slider(layout, Big_slider, label);

    layout.Move_Y(10);
    layout.Size = {70, 20};
    label.text = game.V_Sync ? "V-sync On" : "V-Sync Off";
    layout.Center_animation(VSync_State, false);
    if (Button(layout, Small, label, &VSync_State)) {
        game.V_Sync = !game.V_Sync;
    }
    layout.Center_animation(VSync_State, true);

    layout.Move_Y(10);
    label.text = "???";
    Button(layout, Small, label);

    layout.Move_Y(10);
    label.text = "???";
    Button(layout, Small, label);

    layout.Move_Y(10);
    layout.Size = {150, 20};
    label.text = "Save";
    layout.Center_animation(Save_State, false);
    if (Button(layout, Big, label, &Save_State)) {
        Camera.RenderDistance = renderd;
        game.MenuId = 0; game.Last_Chunk = glm::ivec3(9999);
        glfwSwapInterval(game.V_Sync);
    }
    layout.Center_animation(Save_State, true);

    layout_right.Size = {150, 20};
    layout_right.Move_Y(10);
    layout_right.Size = {70, 20};
    label.text = "???";
    Button(layout_right, Small, label);

    layout_right.Move_Y(10);
    label.text = "???";
    Button(layout_right, Small, label);

    layout_right.Move_Y(10);
    label.text = "???";
    Button(layout_right, Small, label);

    y = layout.Offset.y + layout.Size.y;
}

void Gui::DebugScreen() {
    /*
     FPS + frame time
     [poll | skybox | chunk | remove | tick | mesh | render | gui | bloom | other]
     Legend: color chip + segment name + ms value
     RAM / Triangles progress bars
     Coordinates, facing, block held, target, queues
     */

    struct Src { const char* name; const Timer* timer; uint64_t color; };
    static const Src sources[] = {
        {"Poll",   &PerfS.pollevents, 0xC8B560},
        {"Chunk",  &PerfS.chunk,      0x70C060},
        {"Remove", &PerfS.remove,     0xC07850},
        {"Tick",   &PerfS.tick,       0xB070C0},
        {"Mesh",   &PerfS.mesh,       0x50C0C0},
        {"Render", &PerfS.render,     0xE0C050},
        {"Gui",    &PerfS.gui,        0xD07090},
    };
    constexpr int SourceCount = std::size(sources);

    std::vector<TimeSegment> segments;
    segments.reserve(SourceCount + 1);
    for (const auto &[name, timer, color] : sources)
        segments.push_back({.name = name, .value = std::format("{:.3f}ms", timer->Avg()), .val = timer->Avg(), .color = rgba(color)});

    double accounted = 0.0;
    for (const auto &it : sources)
        accounted += it.timer->Avg();
    const double entire = PerfS.EntireTime.Avg();
    const double other = std::max(0.0, entire - accounted);

    segments.push_back({.name = "Other", .value = std::format("{:.3f}ms", other), .val = other, .color = rgba(0x606060)});

    std::vector<std::string> info;
    info.push_back(std::format("#4FC3F7Pos&r: #90A4AEx&r: #B3E5FC{:.1f} #90A4AEy&r: #B3E5FC{:.1f} #90A4AEz&r: #B3E5FC{:.1f}", Camera.Position.x, Camera.Position.y, Camera.Position.z));
    info.push_back(std::format("#2979FFChunk&r: #90A4AEx&r: #82B1FF{} #90A4AEz&r: #82B1FF{}", Camera.Chunk.x, Camera.Chunk.z));
    info.push_back(std::format("#7C83FFFacing&r: #B39DDB{} #607D8B(#B39DDB{}#607D8B)", Direction_to_String(Camera.direction), Direction_to_Axis(Camera.direction)));
    if (block_cache[Camera.ItemHeld])
        info.push_back(std::format("#66BB6ABlock&r: #A5D6A7{}", block_cache[Camera.ItemHeld]->get_name()));
    if (Camera.looking_at)
        info.push_back(std::format("#EF5350Target&r: #FFCDD2{}", Camera.looking_at->get_name()));
    info.push_back(std::format("#FFB74DMesh&r: #90A4AEpending #FFE0B2{} #90A4AEIn #FFE0B2{} #90A4AEOut #FFE0B2{}", mesher.pendingChunks.size(), mesher.meshQueue.size(), mesher.meshOutQueue.size()));
    info.push_back(std::format("#FFD54FChunk&r: #90A4AEIn #FFE0B2{} #90A4AEOut #FFE0B2{}", GenerateChunk.GenQueue.size(), GenerateChunk.ReadyChunks.size()));

    const auto TextWidth = [](const std::string& text) {
        return MeasureText({.text = text, .Style = {.Scale = 0.5}}).x;
    };

    // Calculate Width
    float content_w = TextWidth(std::format("FPS: {}  {:.2f}ms", game.FPS, entire));
    for (const auto& segment : segments)
        content_w = std::max(content_w, 4.0f + 2.0f + TextWidth(segment.name) + 6.0f + TextWidth(segment.value));
    for (const auto& line : info)
        content_w = std::max(content_w, TextWidth(line));

    static float y = 0;
    const float inner_w = content_w + 2.0f;
    const Layout box = {.Anchor = Anch::TopLeft, .Size = {inner_w + 4.0f, y}, .Offset = {1, 1}};
    Layout layout = {.Anchor = Anch::TopLeft, .Size = {0, 5}, .Offset = {2, 1}, .Parent = &box};

    DrawRectangle(box, {.UV = rgba(0x101010E6)});

    // Header
    Text(Anchor(layout), {.text = std::format("#70C060FPS&r: #B9F6CA{}  #B9F6CA{:.2f}#69F0AEms", game.FPS, entire), .Style = {.Scale = 0.5}});
    layout.Move_Y();

    // Frame time as progress bar: [poll | skybox | chunk | ... | other]
    TimeBar(layout.WithSize({inner_w, 5}), segments.data(), static_cast<int>(segments.size()), static_cast<float>(accounted + other));
    layout.Move_Y(2);

    // Legend
    for (const auto& segment : segments) {
        LegendRow(layout.WithSize({inner_w, 5}), segment, {.name = {.Color = segment.color, .Scale = 0.5}, .value = {.Scale = 0.5}});
        layout.Move_Y();
    }

    // RAM
    static ProgressStyle ram_style = {.TextureId = Texture_Id::None};
    static Label label_ram = {.Style = {.Scale = 0.5}, .anchor = Anch::Center};
    static size_t LastRam;
    const float ramUsedRatio = static_cast<float>(PerfS.ramUsed) / static_cast<float>(game.Max_Ram * 1024 * 1024);
    ram_style.Progress = ramUsedRatio;
    if (LastRam != PerfS.ramUsed) {
        ram_style.BgColor = Gradient(ramUsedRatio, rgba(0x00ff00), rgba(0xffff00), rgba(0xff0000));
        label_ram.text = std::format("{}/{}", Fun::FormatSize(PerfS.ramUsed), Fun::FormatSize(game.Max_Ram * 1024 * 1024));
        LastRam = PerfS.ramUsed;
    }
    layout.Move_Y(2);
    layout.Size = {inner_w, 10};
    ProgressBar(layout, ram_style, &label_ram);

    // Triangles
    static ProgressStyle tri_style = {.TextureId = Texture_Id::None};
    static Label label_tris = {.Style = {.Scale = 0.5}, .anchor = Anch::Center};
    static uint64_t LastTris;
    const float TrisVisibleRatio = PerfS.Total_Triangles ? static_cast<float>(PerfS.Triangles) / static_cast<float>(PerfS.Total_Triangles) : 0.0f;
    tri_style.Progress = TrisVisibleRatio;
    if (LastTris != PerfS.Triangles) {
        tri_style.BgColor = Gradient(TrisVisibleRatio, rgba(0x00ff00), rgba(0xffff00), rgba(0xff0000));
        label_tris.text = std::format("{}/{}", Fun::FormatNumber(PerfS.Triangles), Fun::FormatNumber(PerfS.Total_Triangles));
        LastTris = PerfS.Triangles;
    }
    layout.Move_Y(2);
    ProgressBar(layout, tri_style, &label_tris);

    // World | player info
    layout.Move_Y(2);
    layout.Size = {0, 5};
    for (size_t i = 0; i < info.size(); i++) {
        Text(Anchor(layout), {.text = info[i], .Style = {.Scale = 0.5}});
        if (i + 1 < info.size())
            layout.Move_Y();
    }

    y = layout.Offset.y + layout.Size.y + 1;
}

void Gui::Chat() {
    static int x = 0;
    Label label = {.Style = {.Scale = 0.5}, .anchor = Anch::LeftCenter};
    constexpr Layout box = {Anch::LeftCenter, {125, 100}, {1,50}};
    const Layout input = {Anch::BottomLeft, {125, 10}, {0, 10}, &box};
    Layout text = {Anch::TopLeft, {0, 5}, {1, 1}, &box};
    // Background
    DrawRectangle(box, {{rgba(0x404040D9)}});
    DrawRectangle(input, {{rgba(0x505050D9)}});

    for (const auto& msg : chat) {
        label.text = msg;
        Text(Anchor(text), label);
        text.Move_Y();
    }

    if (InputManager::keysState[GLFW_KEY_ENTER] && net.client.client) {
        const std::string msg = "Hello " + std::to_string(x);
        Packet pkt = {.h = {PacketType::CHAT, static_cast<uint16_t>(msg.size())}};
        pkt.data.resize(msg.size());
        pkt.data.assign(msg.begin(), msg.end());
        memcpy(pkt.data.data(), msg.c_str(), msg.size());
        net.Client_Send(pkt);
        net.client.Send();
        InputManager::keysState[GLFW_KEY_ENTER] = false;
        x++;
    }
    Packet p;
    if (net.Server_Read(p)) {
        if (p.h.type == PacketType::CHAT) {
            net.server.BroadcastPackets(p, nullptr);
        }
    }
    if (net.Client_Read(p)) {
        if (p.h.type == PacketType::CHAT) {
            while (chat.size() > 19)
                chat.erase(chat.begin());
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
