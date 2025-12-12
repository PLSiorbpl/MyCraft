#include <glad/glad.h>
#include "Gui.hpp"

glm::vec4 Gui::Texture(texture tex, int id, int Variant, uint32_t &Flags) {
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

        }
        default: {
            F32.Set(Flags, static_cast<int>(Flag::UseTexture));
            F32.SetTextureId(Flags, 0);
            return glm::vec4(0);
        }
    }
}

glm::vec4 Gui::Color(const glm::vec4 color, uint32_t &Flags) {
    F32.Clear(Flags, static_cast<int>(Flag::UseTexture));
    return color;
}

void Gui::Rectangle(const glm::vec2& Pos, const glm::vec2& Size, const glm::vec4& UV, const uint32_t& Flags) {
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

void Gui::Push(const glm::vec2& Pos, const glm::vec3& UV, const uint32_t& Flags) {
        GuiVertex ver;
        ver.Pos = Pos;
        ver.UV = UV;
        ver.Flags = Flags;
        Mesh.push_back(ver);
}

//glm::vec4 Gui::rgb(uint64_t color) {
//    float a = (color > 0xFFFFFFu) ? (color & 0xFF) / 255.0f : 1.0f;
//    return glm::vec4(
//        ((color >> (color > 0xFFFFFFu ? 24 : 16)) & 0xFF) / 255.0f,
//        ((color >> (color > 0xFFFFFFu ? 16 :  8)) & 0xFF) / 255.0f,
//        ((color >> (color > 0xFFFFFFu ?  8 :  0)) & 0xFF) / 255.0f,
//        a
//    );
//}


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
