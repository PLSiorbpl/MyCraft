#include <glad/glad.h>
#include "Models.hpp"
#include "Chunk.hpp"

std::array<Block *, static_cast<int>(block_type::_count)> block_cache;

void init_block_state() {
    block_cache[static_cast<int>(block_type::Air)]   = new Air();
    block_cache[static_cast<int>(block_type::Stone)] = new Stone();
    block_cache[static_cast<int>(block_type::Grass)] = new Grass();
    block_cache[static_cast<int>(block_type::Dirt)]  = new Dirt();
    block_cache[static_cast<int>(block_type::Iron)]  = new Iron();
    block_cache[static_cast<int>(block_type::Wool)]  = new Wool();
    block_cache[static_cast<int>(block_type::Water)] = new Water();
    block_cache[static_cast<int>(block_type::Lamp)]  = new Lamp();
    block_cache[static_cast<int>(block_type::Redstone_dust)]  = new Redstone_dust();
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    // atexture (location = 1)
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(1);

    // aNormal (location = 2)
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    indexCount = Mesh.size();
    DirtyFlag = false;
}

void Chunk::RemoveData() {
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    Mesh.clear();
    Mesh.shrink_to_fit();
    // if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
    indexCount = 0;
    DirtyFlag = true;
}
