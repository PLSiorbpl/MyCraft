#pragma once
#include <glm/glm.hpp>
#include <cinttypes>
#include <type_traits>
#include <vector>
#include <array>
#include <string>

#include "Utils/Globals.hpp"
#include "Utils/InputManager.hpp"

struct GuiVertex {
    glm::vec2 Pos; // Position
    glm::vec3 UV;  // UV or Color
    uint32_t Flags; // Flags
};

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

enum class texture : int {
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
    texture TextureId = texture::None;
};

struct BoxStyle {
    glm::vec4 BgColor;
    texture TextureId = texture::None;
};

struct ButtonStyle {
    glm::vec4 BgColor = {0.25098f, 0.25098f, 0.25098f, 0.0f};
    glm::vec4 HoverColor = {0.941176f, 0.941176f, 0.941176f, 0.0f};
    texture TextureId = texture::None;
};

struct TextStyle {
    glm::vec4 Color = {0.9647f, 0.9569f, 0.9255f, 0.0f};
    float Scale = 1;
    int PaddingX = 1;
    int PaddingY = 0;
};

struct Label {
    std::string text = "";
    TextStyle Style;
    glm::vec2 Offset = {0.0f, 0.0f};
    Anch anchor = Anch::None;
};

enum class Block : int {
    Stone = 1,
    Grass = 2,
    Dirt = 3,
    Metal = 4,
    Wool = 5,
    Water = 6
};

enum class Flag : int {
    UseTexture = 0,
    TextureId = 1,
};

constexpr uint32_t TextureIdBits = 4u;
constexpr uint32_t TextureIdMask = (1u << TextureIdBits) - 1u; // 0xF

class Flags32 {
public:
    // this from coredumped video on yt xd
    inline void Set(uint32_t &flags, int bit) const {flags |= (1u << bit);}
    inline void Clear(uint32_t &flags, int bit) const {flags &= ~(1u << bit);}
    inline void Toggle(uint32_t &flags, int bit) const {flags ^= (1u << bit);}
    inline bool Get(const uint32_t &flags, int bit) const {return flags & (1u << bit);}
    inline void SetTextureId(uint32_t& flags, int id) {
        id &= TextureIdMask;
        flags &= ~(TextureIdMask << static_cast<uint32_t>(Flag::TextureId));
        flags |= (id << static_cast<uint32_t>(Flag::TextureId));
    }
    inline int GetTextureId(uint32_t flags) {
        return (flags >> static_cast<uint32_t>(Flag::TextureId)) & TextureIdMask;
    }
};

class Gui {
public:
    Flags32 F32;
    std::vector<GuiVertex> Mesh;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei IndexCount;

    glm::vec2 Mouse;
    float width, height;
    static const int Advance[('~'-' ')+1];

    void Clear(const int w, const int h, const float Scale);
    void Send_Data();
    void Push(const glm::vec2& Pos, const glm::vec3& UV, const uint32_t& Flags);
    void Draw();
    void Generate(const int width, const int heigh, const float Scale);

    void DrawRectangle(const Layout& layout, const BoxStyle& style);
    void DrawProgressBar(const Layout& layout, const ProgressStyle& style);

    bool Button(const Layout& layout, const ButtonStyle& style, const Label& label);

    glm::vec2 Anchor(const Layout& layout) const;
    glm::vec2 AnchorText(const glm::vec2& Pos, const glm::vec2& Size, const Label& label);
    glm::vec2 MeasureText(const Label& label);
    glm::vec4 Texture(const texture tex, const glm::vec4 &Size, uint32_t &Flags);
    void Text(const glm::vec2& Pos, const Label& label);
    glm::vec4 Color(const glm::vec4& color, uint32_t &Flags);
    inline const glm::vec4 rgb(const uint64_t color) const {
        const float a = (color > 0xFFFFFFu) ? (color & 0xFF) / 255.0f : 1.0f;
        return glm::vec4(
            ((color >> (color > 0xFFFFFFu ? 24 : 16)) & 0xFF) / 255.0f,
            ((color >> (color > 0xFFFFFFu ? 16 :  8)) & 0xFF) / 255.0f,
            ((color >> (color > 0xFFFFFFu ?  8 :  0)) & 0xFF) / 255.0f,
            a
        );
    }
    inline const int wrap(const int value, const int max) const {
        return (value % max + max) % max;
    };
    inline const bool MouseInRect(const glm::vec2 &Pos, const glm::vec2 &Size) const {
        return (Mouse.x >= Pos.x && Mouse.y >= Pos.y && Mouse.x <= Pos.x+Size.x && Mouse.y <= Pos.y+Size.y);
    }

    void Health();
    void HotBar();
    void Statistics();
    void Menu();
    void Settings();
    void Crosschair();
};