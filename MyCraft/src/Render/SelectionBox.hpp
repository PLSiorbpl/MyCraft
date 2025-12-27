#pragma once
#include <glm/glm.hpp>
#include <vector>

class Selection {
public:
    void Init(const GLuint& id);
    void Draw(const glm::vec3& pos);

    unsigned int vao, vbo;
    GLuint ID;

    std::vector<float> boxLinesCopy;

    const std::vector<float> boxLines = {
        0,0,0, 1,0,0,
        1,0,0, 1,0,1,
        1,0,1, 0,0,1,
        0,0,1, 0,0,0,

        0,1,0, 1,1,0,
        1,1,0, 1,1,1,
        1,1,1, 0,1,1,
        0,1,1, 0,1,0,

        0,0,0, 0,1,0,
        1,0,0, 1,1,0,
        1,0,1, 1,1,1,
        0,0,1, 0,1,1};
};