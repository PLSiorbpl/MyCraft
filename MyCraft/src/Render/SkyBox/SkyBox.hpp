#pragma once
#include <glm/glm.hpp>
#include <Shader_Utils/Shader.hpp>

class SkyBox {
private:
    GLuint vao;

public:
    void Create_SkyBox();
    void Render_SkyBox(const glm::mat4 &invProj, const glm::mat4 &invView, const glm::vec3 &SunDir);
};