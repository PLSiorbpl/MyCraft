#include <glad/glad.h>
#include "Gui.hpp"

glm::vec4 Gui::Texture(texture tex, int id, int Variant, uint32_t &Flags, const glm::vec4 &Size) {
    constexpr float Pixel = 1.0f / 128.0f;
    constexpr float Tile  = 16 * Pixel;

    switch (tex) {
        case texture::Block: {
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
            return (Size / glm::vec4(512));
        }
        default: {
            F32.Set(Flags, static_cast<int>(Flag::UseTexture));
            F32.SetTextureId(Flags, 0);
            return glm::vec4(0);
        }
    }
}

void Gui::Text(const glm::vec2& Pos, const std::string& text, const float Size, uint32_t Flags, const glm::vec2& Padding, const glm::vec2& Offset) {
    F32.Set(Flags, static_cast<int>(Flag::UseTexture));
    F32.SetTextureId(Flags, 2);
    const int Cols = 16;
    const glm::vec2 AtlasS = glm::vec2(128);
    const int glypW = 8;
    const int glypH = 9;
    glm::vec2 Cursor = Pos+Offset;
    for (const char c : text) {
        const int idx = c - ' ';
        const int col = idx % Cols;
        const int row = idx / Cols;
        const glm::vec2 px1 = glm::vec2(col * glypW, row * glypH);
        const glm::vec2 px2 = glm::vec2(px1.x + glypW, px1.y + glypH);
        const glm::vec4 uv = glm::vec4(px1/AtlasS, px2/AtlasS);
        const glm::vec2 wh = glm::vec2((glypW) * Size, glypH * Size);
        DrawRectangle(Cursor, wh, uv, Flags);
        Cursor += glm::vec2(wh.x-Padding.x-Advance[idx], 0);
    }
}

glm::vec4 Gui::Color(const glm::vec4 color, uint32_t &Flags) {
    F32.Clear(Flags, static_cast<int>(Flag::UseTexture));
    return color;
}

void Gui::DrawRectangle(const glm::vec2& Pos, const glm::vec2& Size, const glm::vec4& UV, uint32_t& Flags) {
    if (F32.Get(Flags, static_cast<int>(Flag::UseTexture))) {
        Push(Pos, {UV.x, UV.y, 0}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.z, UV.y, 0}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.w, 0}, Flags);

        Push(Pos + Size, {UV.z, UV.w, 0}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.z, UV.y, 0}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.w, 0}, Flags);
    } else { // Solid Color
        Push(Pos, {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.y, UV.z}, Flags);

        Push(Pos + Size, {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(Size.x, 0.0f), {UV.x, UV.y, UV.z}, Flags);
        Push(Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.y, UV.z}, Flags);
    }
}

void Gui::DrawProgressBar(float Progress, const glm::vec2 &Pos, const glm::vec2 &Size, const glm::vec4 &UV, uint32_t &Flags) {
    Progress = glm::clamp(Progress, 0.0f, 1.0f);
    const float filledWidth = Size.x * Progress;
    glm::vec4 uv = UV;
    if (F32.Get(Flags, static_cast<int>(Flag::UseTexture))) {
        uv.z = uv.x + (UV.z - UV.x) * Progress;
    }
    DrawRectangle(Pos, {filledWidth, Size.y}, uv, Flags);
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

glm::vec2 Gui::Anchor(Anch anchor, const glm::vec2 &Size, glm::vec2 Offset) const {
    switch (anchor) {
        case Anch::BottomLeft:   return glm::vec2(0, height - Size.y) + Offset;
        case Anch::BottomRight:  return glm::vec2(width - Size.x, height - Size.y) + Offset;
        case Anch::TopLeft:      return glm::vec2(0, 0) + Offset;
        case Anch::TopRight:     return glm::vec2(width - Size.x, 0) + Offset;
        case Anch::Center:       return glm::vec2((width - Size.x)/2, (height - Size.y)/2) + Offset;
        case Anch::BottomCenter: return glm::vec2((width-Size.x)/2, height-Size.y) + Offset;
        case Anch::TopCenter:    return glm::vec2((width-Size.x)/2, 0) + Offset;
        case Anch::LeftCenter:   return glm::vec2(0, (height - Size.y)/2) + Offset;
        case Anch::RightCenter:  return glm::vec2(width-Size.x, (height - Size.y)/2) + Offset;
        case Anch::Full:         return glm::vec2(0,0);
    }
    return glm::vec2(0);
}

bool Gui::Button(const glm::vec2& pos, const glm::vec2& size, uint32_t flags, int Mode, const glm::vec4 &Color_1, const glm::vec4 &Color_2) {
    bool hovered = MouseInRect(pos, size);
    glm::vec4 col = hovered ? Color_2 : Color_1;
    if (Mode == 0) col = Color(col, flags);

    DrawRectangle(pos, size, col, flags);

    return hovered && In.MouseState[GLFW_MOUSE_BUTTON_1];
}

const int Gui::Advance[('~'-' ')+1] = {
    /* ' ' */ 0,
    /* '!' */ 3,
    /* '"' */ 1,
    /* '#' */ 0,
    /* '$' */ 0,
    /* '%' */ 0,
    /* '&' */ -1,
    /* ''' */ 2,
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
    /* ':' */ 3,
    /* ';' */ 3,
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
    /* 'O' */ 0,
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
    /* 'l' */ 3,
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