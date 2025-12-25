#include <glad/glad.h>
#include "Gui.hpp"

glm::vec4 Gui::Texture(const texture tex, const glm::vec4 &UV, uint32_t &Flags) {
    constexpr float Pixel = 1.0f / 128.0f;
    constexpr float Tile  = 16 * Pixel;

    switch (tex) {
        case texture::Block: {
            int id = UV.x;
            const int Variant = UV.y;
            F32.Set(Flags, static_cast<int>(Flag::UseTexture));
            F32.SetTextureId(Flags, 0);
            id %= 6;
            return glm::vec4(id*Tile, Variant*Tile, (id+1)*Tile, (Variant+1)*Tile);
        }
        case texture::Item: {

        }
        case texture::Gui: {
            F32.Set(Flags, static_cast<int>(Flag::UseTexture));
            F32.SetTextureId(Flags, 1);
            return (UV / glm::vec4(512));
        }
        case texture::Font: {
            F32.Set(Flags, static_cast<int>(Flag::UseTexture));
            F32.SetTextureId(Flags, 2);
            return UV;
        }
        default: {
            F32.Set(Flags, static_cast<int>(Flag::UseTexture));
            F32.SetTextureId(Flags, 0);
            return glm::vec4(0);
        }
    }
}

void Gui::Text(const glm::vec2& Pos, const Label& label) {
    constexpr int Cols = 16;
    constexpr glm::vec2 AtlasS = glm::vec2(128);
    constexpr int glypW = 8;
    constexpr int glypH = 9;
    glm::vec2 Cursor = Pos + label.Offset;
    for (unsigned char c : label.text) {
        if (c < ' ' || c > '~') c = '?';
        const int idx = c - ' ';
        const int col = idx % Cols;
        const int row = idx / Cols;
        const glm::vec2 px1 = glm::vec2(col * glypW, row * glypH);
        const glm::vec2 px2 = glm::vec2(px1.x + glypW, px1.y + glypH);
        const glm::vec4 uv = glm::vec4((px1/AtlasS), px2/AtlasS);
        const glm::vec2 Size = glm::vec2((glypW * label.Style.Scale), (glypH * label.Style.Scale));
        DrawRectangle(
            {.Anchor = Anch::None, .Size = Size, .Offset = Cursor},
            {.BgColor = uv, .TextureId = texture::Font}
        );
        Cursor += glm::vec2(Size.x - (label.Style.PaddingX * label.Style.Scale) - (Advance[idx] * label.Style.Scale), 0);
    }
}

bool Gui::UpdateText(TextCache& c, int v, const char* fmt) {
    if (c.i == v) return false;
    c.i = v;

    char buf[128];
    std::snprintf(buf, 64, fmt, v);
    c.text = buf;
    return true;
}

bool Gui::UpdateText(TextCache& cache, float value, const char* fmt) {
    if (std::abs(cache.f - value) < 1e-5f) return false;
    cache.f = value;
    char buf[128];
    std::snprintf(buf, sizeof(buf), fmt, value);
    cache.text = buf;
    return true;
}

bool Gui::UpdateText(TextCache& cache, double value, const char* fmt) {
    if (std::abs(cache.d - value) < 1e-9) return false;
    cache.d = value;
    char buf[128];
    std::snprintf(buf, sizeof(buf), fmt, value);
    cache.text = buf;
    return true;
}

std::string Gui::Format(const char* fmt, ...) {
    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    return std::string(buffer);
}

glm::vec4 Gui::Color(const glm::vec4& color, uint32_t &Flags) {
    F32.Clear(Flags, static_cast<int>(Flag::UseTexture));
    return color;
}

void Gui::DrawRectangle(const Layout& layout, const BoxStyle& style) {
    uint32_t Flags = 0;
    const glm::vec2 Pos = Anchor(layout);
    const glm::vec2 Size = layout.Size;

    glm::vec4 UV;
    if (style.TextureId == texture::None) {
        // Solid Color
        UV = Color(style.BgColor, Flags);
        Push(Pos, {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.y, UV.z}, Flags);

        Push(Pos + Size, {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.y, UV.z}, Flags);
    } else {
        // Texture
        UV = Texture(style.TextureId, style.BgColor, Flags);
        Push(Pos, {UV.x, UV.y, 0}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.z, UV.y, 0}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.w, 0}, Flags);

        Push(Pos + Size, {UV.z, UV.w, 0}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.z, UV.y, 0}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.w, 0}, Flags);
    }
}

void Gui::DrawProgressBar(const Layout& layout, const ProgressStyle& style, Label* label) {
    const glm::vec2 Pos = Anchor(layout);
    const float Progress = glm::clamp(style.Progress, 0.0f, 1.0f);
    const float filledWidth = layout.Size.x * Progress;

    DrawRectangle(
        {.Anchor = layout.Anchor, .Size = {filledWidth, layout.Size.y}, .Offset = layout.Offset},
        {.BgColor = style.BgColor, .TextureId = style.TextureId}
    );
    if (label != nullptr) {
        const glm::vec2 TextPos = AnchorText(Pos, layout.Size, *label);
        Text(TextPos, *label);
    }
}

void Gui::Push(const glm::vec2& Pos, const glm::vec3& UV, const uint32_t& Flags) {
        GuiVertex ver;
        ver.Pos = Pos;
        ver.UV = UV;
        ver.Flags = Flags;
        Mesh.push_back(ver);
}

void Gui::Send_Data() {
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // aPos (location = 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), (void*)offsetof(GuiVertex, Pos));
        glEnableVertexAttribArray(0);

        // atexture (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), (void*)offsetof(GuiVertex, UV));
        glEnableVertexAttribArray(1);

        // aFlags (location = 2)
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(GuiVertex), (void*)offsetof(GuiVertex, Flags));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, Mesh.size() * sizeof(GuiVertex), Mesh.data(), GL_STATIC_DRAW);

    IndexCount = Mesh.size();
}

void Gui::Draw() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, IndexCount);
    
    // Render ImGui
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Gui::Clear(const int w, const int h, const float Scale) {
    Mouse = glm::vec2(
        In.MouseX/Scale,
        In.MouseY/Scale
    );
    width = w;
    height = h;
    Mesh.clear();
}

glm::vec2 Gui::Anchor(const Layout& layout) const {
    const glm::vec2 Size = layout.Size;
    const glm::vec2 Offset = layout.Offset;
    switch (layout.Anchor) {
        case Anch::BottomLeft:   return glm::vec2(0, height - Size.y) + Offset;
        case Anch::BottomRight:  return glm::vec2(width - Size.x, height - Size.y) + Offset;
        case Anch::TopLeft:      return glm::vec2(0, 0) + Offset;
        case Anch::TopRight:     return glm::vec2(width - Size.x, 0) + Offset;
        case Anch::Center:       return glm::vec2((width - Size.x)/2, (height - Size.y)/2) + Offset;
        case Anch::BottomCenter: return glm::vec2((width-Size.x)/2, height-Size.y) + Offset;
        case Anch::TopCenter:    return glm::vec2((width-Size.x)/2, 0) + Offset;
        case Anch::LeftCenter:   return glm::vec2(0, (height - Size.y)/2) + Offset;
        case Anch::RightCenter:  return glm::vec2(width-Size.x, (height - Size.y)/2) + Offset;
        case Anch::None:         return glm::vec2(Offset);
    }
    return glm::vec2(0);
}

glm::vec2 Gui::MeasureText(const Label& label) {
    constexpr int glypW = 8;
    constexpr int glypH = 9;
    glm::vec2 TextSize = {0, glypH * label.Style.Scale};
    for (unsigned char c : label.text) {
        if (c < ' ' || c > '~') c = '?';
        const int idx = c - ' ';
        const glm::vec2 Size = glm::vec2(glypW * label.Style.Scale, glypH * label.Style.Scale);
        TextSize.x += Size.x - (label.Style.PaddingX * label.Style.Scale) - (Advance[idx] * label.Style.Scale);
    }
    return TextSize;
}

glm::vec2 Gui::AnchorText(const glm::vec2& Pos, const glm::vec2& Size, const Label& label) {
    const glm::vec2 TextSize = MeasureText(label);
    constexpr glm::vec2 TextOffset = {0.0f, -1.0f};
    const glm::vec2 Offset = Pos + label.Offset + TextOffset;
    switch (label.anchor) {
        case Anch::BottomLeft:   return glm::vec2(0, Size.y - TextSize.y) + Offset;
        case Anch::BottomRight:  return glm::vec2(Size.x - TextSize.x, Size.y - TextSize.y) + Offset;
        case Anch::TopLeft:      return glm::vec2(0, 0) + Offset;
        case Anch::TopRight:     return glm::vec2(Size.x - TextSize.x, 0) + Offset;
        case Anch::Center:       return glm::vec2((Size.x - TextSize.x)/2, (Size.y - TextSize.y)/2) + Offset;
        case Anch::BottomCenter: return glm::vec2((Size.x-TextSize.x)/2, Size.y-TextSize.y) + Offset;
        case Anch::TopCenter:    return glm::vec2((Size.x-TextSize.x)/2, 0) + Offset;
        case Anch::LeftCenter:   return glm::vec2(0, (Size.y - TextSize.y)/2) + Offset;
        case Anch::RightCenter:  return glm::vec2(Size.x-TextSize.x, (Size.y - TextSize.y)/2) + Offset;
        case Anch::None:         return glm::vec2(Offset);
    }
    return glm::vec2(0);
}

bool Gui::Button(const Layout& layout, const ButtonStyle& style, const Label& label) {
    const int id = ID;
    ID += 1;
    bool clicked = false;
    const glm::vec2 Pos = Anchor(layout);
    const bool hover = MouseInRect(Pos, layout.Size);

    glm::vec4 Col = hover ? style.HoverColor : style.BgColor;
    
    if (hover && In.MouseState[GLFW_MOUSE_BUTTON_1] && ActiveId == -1) {
        ActiveId = id;
    }
    if (!In.MouseState[GLFW_MOUSE_BUTTON_1]) {
        if (ActiveId == id && hover)
            clicked = true;

        if (ActiveId == id)
            ActiveId = -1;
    }

    DrawRectangle(
        layout,
        {.BgColor = Col, .TextureId = style.TextureId}
    );
    const glm::vec2 TextPos = AnchorText(Pos, layout.Size, label);
    Text(TextPos, label);
    
    return clicked;
}

glm::vec4 Gui::Gradient(float x, const glm::vec4& a, const glm::vec4& b) {
    x = glm::clamp(x, 0.0f, 1.0f);
    return glm::mix(a, b, x);
}

glm::vec4 Gui::Gradient(float x, const glm::vec4& a, const glm::vec4& b, const glm::vec4& c) {
    x = glm::clamp(x, 0.0f, 1.0f);
    if (x < 0.5f)
        return glm::mix(a, b, x * 2.0f);
    else
        return glm::mix(b, c, (x - 0.5f) * 2.0f);
}

const int Gui::Advance[('~'-' ')+1] = {
    /* ' ' */ 1,
    /* '!' */ 4,
    /* '"' */ 2,
    /* '#' */ 0,
    /* '$' */ 0,
    /* '%' */ 0,
    /* '&' */ -1,
    /* ''' */ 3,
    /* '(' */ 2,
    /* ')' */ 2,
    /* '*' */ 2,
    /* '+' */ 2,
    /* ',' */ 3,
    /* '-' */ 2,
    /* '.' */ 4,
    /* '/' */ 1,
    /* '0' */ 0,
    /* '1' */ 0,
    /* '2' */ 0,
    /* '3' */ 0,
    /* '4' */ 0,
    /* '5' */ 0,
    /* '6' */ 0,
    /* '7' */ 0,
    /* '8' */ 0,
    /* '9' */ 0,
    /* ':' */ 4,
    /* ';' */ 4,
    /* '<' */ 2,
    /* '=' */ 2,
    /* '>' */ 2,
    /* '?' */ 0,
    /* '@' */ 0,
    /* 'A' */ 0,
    /* 'B' */ 0,
    /* 'C' */ 0,
    /* 'D' */ 0,
    /* 'E' */ 1,
    /* 'F' */ 1,
    /* 'G' */ 0,
    /* 'H' */ 0,
    /* 'I' */ 2,
    /* 'J' */ 1,
    /* 'K' */ 1,
    /* 'L' */ 1,
    /* 'M' */ 0,
    /* 'N' */ 0,
    /* 'O' */ -1,
    /* 'P' */ 0,
    /* 'Q' */ -1,
    /* 'R' */ 0,
    /* 'S' */ 0,
    /* 'T' */ 0,
    /* 'U' */ 0,
    /* 'V' */ 0,
    /* 'W' */ 0,
    /* 'X' */ 0,
    /* 'Y' */ 0,
    /* 'Z' */ 0,
    /* '[' */ 2,
    /* '\' */ 1,
    /* ']' */ 2,
    /* '^' */ 0,
    /* '_' */ 0,
    /* '`' */ 3,
    /* 'a' */ 0,
    /* 'b' */ 1,
    /* 'c' */ 2,
    /* 'd' */ 1,
    /* 'e' */ 1,
    /* 'f' */ 1,
    /* 'g' */ 1,
    /* 'h' */ 2,
    /* 'i' */ 4,
    /* 'j' */ 2,
    /* 'k' */ 1,
    /* 'l' */ 4,
    /* 'm' */ 0,
    /* 'n' */ 2,
    /* 'o' */ 1,
    /* 'p' */ 1,
    /* 'q' */ 1,
    /* 'r' */ 1,
    /* 's' */ 1,
    /* 't' */ 2,
    /* 'u' */ 1,
    /* 'v' */ 0,
    /* 'w' */ 0,
    /* 'x' */ 0,
    /* 'y' */ 1,
    /* 'z' */ 0,
    /* '{' */ 3,
    /* '|' */ 3,
    /* '}' */ 3,
    /* '~' */ -1
};