#include "GuiBackend.hpp"
#include "Utils/Globals.hpp"
#include "Shader.hpp"
#include "Utils/InputManager.hpp"

using namespace gui;

void GuiBackend::ResetFrame() {
    const int ScaleX = game_settings.width  / 320;
    const int ScaleY = game_settings.height / 240;

    game_settings.Scale = std::min(ScaleX, ScaleY);
    if (game_settings.Scale < 1)
        game_settings.Scale = 1;
    game_settings.Scaled_w = game_settings.width/game_settings.Scale;
    game_settings.Scaled_h = game_settings.height/game_settings.Scale;

    game_settings.Mouse = glm::vec2(
        InputManager::MouseX/game_settings.Scale,
        InputManager::MouseY/game_settings.Scale);

    Mesh.clear();

    projection = glm::ortho(0.0f, static_cast<float>(game_settings.Scaled_w),
            static_cast<float>(game_settings.Scaled_h), 0.0f, -1.0f, 1.0f);
}

void GuiBackend::SendMesh() {
    glUseProgram(SH.General_Gui_Shader.Shader);
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // aPos (location = 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), static_cast<void *>(nullptr));
        glEnableVertexAttribArray(0);

        // atexture (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), reinterpret_cast<void *>(offsetof(GuiVertex, UV)));
        glEnableVertexAttribArray(1);

        // aFlags (location = 2)
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(GuiVertex), reinterpret_cast<void *>(offsetof(GuiVertex, Flags)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, Mesh.size() * sizeof(GuiVertex), Mesh.data(), GL_STATIC_DRAW);

    IndexCount = Mesh.size();
}

void GuiBackend::RenderFrame() const {
    glUseProgram(SH.General_Gui_Shader.Shader);

    Shader::Set_Int(SH.General_Gui_Shader.Shader, "BaseTexture", 0);
    Shader::Set_Int(SH.General_Gui_Shader.Shader, "GuiTexture", 1);
    Shader::Set_Int(SH.General_Gui_Shader.Shader, "FontTexture", 2);

    Shader::Set_Mat4(SH.General_Gui_Shader.Shader, "Model", model);
    Shader::Set_Mat4(SH.General_Gui_Shader.Shader, "Projection", projection);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, IndexCount);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void GuiBackend::PushToMesh(const gui::GuiVertex &vertex) {
    Mesh.push_back(vertex);
}
