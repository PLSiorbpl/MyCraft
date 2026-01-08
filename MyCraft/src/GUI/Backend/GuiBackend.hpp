#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "glm/ext/matrix_clip_space.hpp"
#include <vector>
#include "../Common/Gui_Types.hpp"

class GuiBackend {
public:
    std::vector<gui::GuiVertex> Mesh;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei IndexCount = 0;

    static constexpr auto model = glm::mat4(1.0f);
    glm::mat4 projection;

    void ResetFrame();
    void SendMesh();
    void RenderFrame() const;

    void PushToMesh(const gui::GuiVertex &vertex);
};
