#include <glad/glad.h>
#include "Chunk.hpp"


const std::map<uint8_t, Chunk::Block> Chunk::BlockDefs = {
    { 0, Block(0, false, false) }, // Air
    { 1, Block(1, false, true) }, // Stone
    { 2, Block(2, false,  true) }, // Grass
    { 3, Block(3, false,  true) }, // Dirt
    { 4, Block(4, false, true) }, // Blacha
    { 5, Block(5, false, true) }, // Wool
    { 6, Block(6, true, true) } // Water
 };

void Chunk::Allocate() {
    const size_t estimate = static_cast<size_t>(width) * height * depth;
    Alloc = static_cast<size_t>(estimate * 0.75);
    Mesh.clear();
    Mesh.reserve(Alloc);
}

void Chunk::SendData() {
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        //glGenBuffers(1, &ebo);
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, Mesh.size() * sizeof(Vertex), Mesh.data(), GL_STATIC_DRAW);

    // aPos (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // atexture (location = 1)
    glVertexAttribIPointer(1, 2, GL_UNSIGNED_BYTE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(1);

    // aNormal (location = 2)
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    indexCount = Mesh.size();
    DirtyFlag = false;
}

void Chunk::RemoveData() {
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    // if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
    indexCount = 0;
    DirtyFlag = true;
}
