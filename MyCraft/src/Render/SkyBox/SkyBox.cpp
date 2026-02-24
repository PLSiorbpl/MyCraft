#include "SkyBox.hpp"
#include "Utils/Globals.hpp"
#include "Shader_Utils/Shader.hpp"

void SkyBox::Create_SkyBox() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindVertexArray(0);
}

void SkyBox::Render_SkyBox(const glm::mat4 &invProj, const glm::mat4 &invView, const glm::vec3 &SunDir) {
    glUseProgram(SH.SkyBox_Shader.Shader);

    const float dayFactor = glm::clamp(SunDir.y * 0.5f + 0.5f, 0.0f, 1.0f);

    Shader::Set_Mat4(SH.SkyBox_Shader.Shader, "invView", invView);
    Shader::Set_Mat4(SH.SkyBox_Shader.Shader, "invProj", invProj);
    Shader::Set_Vec3(SH.SkyBox_Shader.Shader, "sunDir", SunDir);
    Shader::Set_Float(SH.SkyBox_Shader.Shader, "dayfactor", dayFactor);

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}