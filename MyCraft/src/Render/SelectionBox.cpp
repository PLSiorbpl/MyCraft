#include <glad/glad.h>
#include "SelectionBox.hpp"

void Selection::Init(const GLuint& id) {
    ID = id;
    glUseProgram(ID);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 72, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
}

void Selection::Draw(const glm::vec3& pos) {
    float offset = 0.001f;
    boxLinesCopy = boxLines;
    for (int i = 0; i < 72; i += 3) {
        boxLinesCopy[i]   += pos.x + (boxLinesCopy[i]   > 0 ? offset : -offset);
        boxLinesCopy[i+1] += pos.y + (boxLinesCopy[i+1] > 0 ? offset : -offset);
        boxLinesCopy[i+2] += pos.z + (boxLinesCopy[i+2] > 0 ? offset : -offset);
    }
}
