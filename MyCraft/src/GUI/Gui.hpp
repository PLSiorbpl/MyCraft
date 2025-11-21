#pragma once
#include <glm/glm.hpp>
#include <cinttypes>
#include <vector>
#include <array>

struct GuiVertex {
    glm::vec2 Pos;
    glm::vec2 UV;
    uint32_t Special;
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
    RightCenter
};

class Gui {
public:
    std::vector<GuiVertex> Mesh;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei IndexCount;

    float width, height;

    void Clear(int w, int h);
    void Send_Data();
    void Push(const glm::vec2& Pos, const glm::vec2& UV, const uint64_t& Special);

    void Rectangle(const glm::vec2& Pos, const glm::vec2& Size, const glm::vec2& UV, const uint64_t& Special);
    glm::vec2 Anchor(Anch anchor, const glm::vec2 &Size, glm::vec2 Offset = {0.0f,0.0f});

    void HotBar();
    void Statistics();
    void Menu();
    void Crosschair();
};