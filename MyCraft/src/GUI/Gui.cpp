#include <glad/glad.h>
#include "Gui.hpp"

void Gui::HotBar() {
    uint64_t Special = 0;
    glm::vec2 UV = glm::vec2(0.0);

    const float slotSize = 20.0f;
    const float slotSpacing = 1.0f + slotSize;

    glm::vec2 Size = glm::vec2(190.0f, 22.0f);
    Rectangle(Anchor(Anch::BottomCenter, Size, {0.0f, -2.0f}), Size, UV, Special);

    Size = glm::vec2(slotSize);
    for (int i = 0; i < 9; i++) {
        Rectangle(Anchor(Anch::BottomCenter, Size, {-84.0f+(i*slotSpacing), -3.0f}), Size, UV, Special);
    }
}

void Gui::Statistics() {
    uint64_t Special = 0;
    glm::vec2 UV = glm::vec2(0.0);

    // Health
    float HalfHotBar = 188.0f/2;
    glm::vec2 StatSize = glm::vec2(8.5f, 8.5f);
    float StatSpacing = 0.5f + StatSize.x;

    for (int i = 0; i < 10; i++) {
        Rectangle(Anchor(Anch::BottomCenter, StatSize, {-HalfHotBar + StatSize.x/2 + (i*StatSpacing), -26.0f}), StatSize, UV, Special);
    }

    // Food
    for (int i = 0; i < 10; i++) {
        Rectangle(Anchor(Anch::BottomCenter, StatSize, {0.0f + StatSize.x + (i*StatSpacing), -26.0f}), StatSize, UV, Special);
    }

    // Water
    for (int i = 0; i < 10; i++) {
        Rectangle(Anchor(Anch::BottomCenter, StatSize, {0.0f + StatSize.x + (i*StatSpacing), -36.5f}), StatSize, UV, Special);
    }
}

glm::vec2 Gui::Anchor(Anch anchor, const glm::vec2 &Size, glm::vec2 Offset) {
    switch(anchor) {
        case Anch::BottomLeft:   return glm::vec2(0, height - Size.y) + Offset;
        case Anch::BottomRight:  return glm::vec2(width - Size.x, height - Size.y) + Offset;
        case Anch::TopLeft:      return glm::vec2(0, 0) + Offset;
        case Anch::TopRight:     return glm::vec2(width - Size.x, 0) + Offset;
        case Anch::Center:       return glm::vec2((width - Size.x)/2, (height - Size.y)/2) + Offset;
        case Anch::BottomCenter: return glm::vec2((width-Size.x)/2, height-Size.y) + Offset;
        case Anch::TopCenter:   return glm::vec2((width-Size.x)/2, 0) + Offset;
        case Anch::LeftCenter:   return glm::vec2(0, (height - Size.y)/2) + Offset;
        case Anch::RightCenter:  return glm::vec2(width-Size.x, (height - Size.y)/2) + Offset;
    }
    return glm::vec2(0);
}

void Gui::Rectangle(const glm::vec2& Pos, const glm::vec2& Size, const glm::vec2& UV, const uint64_t& Special) {
    Push(Pos, UV, Special);
    Push(Pos + glm::vec2(Size.x, 0.0f), UV, Special);
    Push(Pos + glm::vec2(0.0f, Size.y), UV, Special);

    Push(Pos + Size, UV, Special);
    Push(Pos + glm::vec2(Size.x, 0.0f), UV, Special);
    Push(Pos + glm::vec2(0.0f, Size.y), UV, Special);
}

void Gui::Push(const glm::vec2& Pos, const glm::vec2& UV, const uint64_t& Special) {
        GuiVertex ver;
        ver.Pos = Pos;
        ver.UV = UV;
        ver.Special = Special;
        Mesh.push_back(ver);
}

void Gui::Clear(int w, int h) {
    width = w;
    height = h;
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    Mesh.clear();
}

void Gui::Send_Data() {
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, Mesh.size() * sizeof(GuiVertex), Mesh.data(), GL_STATIC_DRAW);

    // aPos (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), (void*)offsetof(GuiVertex, Pos));
    glEnableVertexAttribArray(0);

    // atexture (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), (void*)offsetof(GuiVertex, UV));
    glEnableVertexAttribArray(1);

    // aNormal (location = 2)
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(GuiVertex), (void*)offsetof(GuiVertex, Special));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    IndexCount = Mesh.size();
}