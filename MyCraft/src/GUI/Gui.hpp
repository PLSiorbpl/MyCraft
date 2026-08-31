#pragma once
#include <glm/glm.hpp>
#include <cinttypes>
#include <vector>
#include <string>

#include "Utils/Globals.hpp"
#include "Utils/Function.hpp"
#include "Common/Gui_Types.hpp"
#include "Backend/GuiBackend.hpp"
#include "Common/Flags.hpp"

// fuck this too boring
/*
Provides:
    - API for building primitive GUI elements and widgets
    - Layout, anchoring, color and texture helpers
    - Text formatting, measurement and caching utilities
    - High-level screens (HotBar, Menu, Performance, etc.)

Responsibility:
    - Builds GUI mesh every "frame"
    - Manages widget state
    - Handles interactions with GUI elements
    - Computes positions of GUI elements related to Anchor
    - Controls GUI-related shader flags and parameters

Does Not:
    - Perform actual rendering (Handled by GuiBackend)
    - Store persistent widget objects
    - Manage windowing or low-level input

Note:
    - Inspired by Dear ImGui (immediate-mode approach)
    - Entire GUI is remeshed every frame by design
    - Favors clarity and control over strict abstraction
 */

enum class Gui_State {
    None,
    Chat,
    Health,
    Menu,
    Inventory,
};

class Gui {
public:
    // State
    GuiBackend backend;
    Flags32 F32;
    Fun fun;

    Gui_State gui_state;
    int ActiveId = 0;
    int ID = 0;
    static const int Advance[('~'-' ')+1];
    static std::vector<std::string> chat;

    // Frame
    void Generate();
    void Update();

    // Primitive
    void DrawRectangle(const gui::Layout& layout, const gui::BoxStyle& style);

    // Widgets
    bool Button(const gui::Layout &layout, const gui::ButtonStyle &style, const gui::Label &label, gui::Animation_State<glm::vec2>* state = nullptr);
    void ProgressBar(const gui::Layout& layout, const gui::ProgressStyle& style, const gui::Label* label = nullptr);
    void TimeBar(const gui::Layout& layout, const gui::TimeSegment* segments, int count, double total);
    void LegendRow(const gui::Layout& row, const gui::TimeSegment& segment, const gui::LegendStyle &style);
    void Text(const glm::vec2& Pos, const gui::Label& label);
    bool TextInput(const gui::Layout& layout, const gui::TextInputStyle& style, gui::Label& label, gui::Animation_State<glm::vec2>* state = nullptr);
    void Slider(const gui::Layout& layout, gui::SliderStyle &style, const gui::Label& label);

    // Layout | Metrics
    [[nodiscard]] static glm::vec2 Anchor(const gui::Layout& layout);
    static glm::vec2 AnchorText(const glm::vec2& Pos, const glm::vec2& Size, const gui::Label& label);
    static glm::vec2 MeasureText(const gui::Label& label);

    // Texture | Color | Shading
    static glm::vec4 Texture(gui::Texture_Id tex, const glm::vec4 &Size, uint32_t &Flags);
    static glm::vec4 Gradient(float x, const glm::vec4& a, const glm::vec4& b);
    static glm::vec4 Gradient(float x, const glm::vec4& a, const glm::vec4& b, const glm::vec4& c);
    static glm::vec4 Color(const glm::vec4& color, uint32_t &Flags);

    // Screens
    void Health();
    void HotBar();
    void Food_bar();
    void Menu();
    void Settings();
    void Chat();
    void Multiplayer();
    void MultiplayerJoin();
    void MultiplayerHost();
    void Crosschair();
    void DebugScreen();
    void Inventory();
};