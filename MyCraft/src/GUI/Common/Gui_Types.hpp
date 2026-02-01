#pragma once
#include <glm/glm.hpp>
#include <cinttypes>
#include <string>
#include "Utils/Interpolater.hpp"

/*
Contains:
    - data-only types used by GUI system
    - no logic, rendering or input handling

Responsibility:
    - Layout descriptions
    - Style definitions
    - Gui Vertex format
    - enums used by Gui

Note:
    - This file should only include data-only types and no heavy logic
    - Default values can make code cleaner ex: int Offset = 0; so
      Dev can choose if he wants to use Offset
 */

namespace gui {
    struct GuiVertex {
        /*
            Pos - 2d Position of vertex
            UV - Holds Texture UV or RGB color depending on flag '0'
            Flags - Represents Flags by using individual bits to save space on gpu
         */
        glm::vec2 Pos;
        glm::vec3 UV;
        uint32_t Flags;
        /*
            Flags Layout: (bits)
            0 - Describes if shader uses texture or solid color
            1-4 - Texture Id (see enum Texture_Id)
         */
    };
    // Anchor Helps position the gui on screen relative to screen or parent
    enum class Anch {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center,
        BottomCenter,
        TopCenter,
        LeftCenter,
        RightCenter,
        None
    };
    // Used by shader to select texture atlas (0–4), packed into GuiVertex::Flags
    enum class Texture_Id : int {
        None = 0,
        Block = 1,
        Item = 2,
        Gui = 3,
        Font = 4
    };

    struct Layout  {
        Anch Anchor;
        glm::vec2 Size;
        glm::vec2 Offset = {0.0f,0.0f};
    };

    struct ProgressStyle {
        float Progress = 0;
        glm::vec4 BgColor;
        Texture_Id TextureId = Texture_Id::None;
    };

    struct BoxStyle {
        glm::vec4 BgColor;
        Texture_Id TextureId = Texture_Id::None;
    };

    enum class State {
        Idle,
        Hover,
        Click,
    };

    template<typename T>
    struct Animation_State {
        Interpolate<T> inter{};
        State state = State::Idle;
    };

    struct ButtonStyle {
        glm::vec4 BgColor = {0.25098f, 0.25098f, 0.25098f, 0.0f};
        glm::vec4 HoverColor = {0.941176f, 0.941176f, 0.941176f, 0.0f};
        Texture_Id TextureId = Texture_Id::None;
        bool show = true;
    };

    struct TextInputStyle {
        ButtonStyle style;
        std::string Default_str;
        char Cursor = '_';
        float timing = 0.5f;
        unsigned int max_chars = 24;
        /*
            0 - Any
            1 - Numbers Only
         */
        unsigned int Input_Mode = 0;
    };

    struct TextStyle {
        glm::vec4 Color = {0.9647f, 0.9569f, 0.9255f, 0.0f};
        float Scale = 1;
        int PaddingX = 1;
        int PaddingY = 0;
    };

    struct Label {
        std::string text;
        TextStyle Style = {{0.9647f, 0.9569f, 0.9255f, 0.0f}, 1, 1, 0};
        glm::vec2 Offset = {0.0f, 0.0f};
        Anch anchor = Anch::None;
    };

    struct TextCache {
        /*
            stores last value to prevent rebuilding string every frame
         */
        int i = 0;
        float f = 0.0001f;
        double d = std::numeric_limits<double>::quiet_NaN();
        std::string text;
    };

    enum class FlagBit : int {
        UseTexture = 0,
        TextureId = 1,
    };
}