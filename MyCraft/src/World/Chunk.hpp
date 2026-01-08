#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>
#include <cstdint>

typedef unsigned int GLuint;
typedef int GLsizei;

class Chunk {
public:
    struct Block {
        uint8_t id;
        // Flags: Solid, Transparent, ...
        uint8_t Flags;

        explicit Block(const uint8_t id = 0, const uint8_t Flags = 0)
            : id(id), Flags(Flags) {}
    };

    struct Vertex {
        glm::vec3 position;
        uint8_t uv[2];
        uint8_t normal;
        uint8_t pad = 0;
    };

    // World Stuff
    std::vector<Block> blocks;
    int width, height, depth;
    int chunkX, chunkZ;
    static const std::map<uint8_t, Block> BlockDefs;

    // Mesh Stuff
    bool InRender = false;
    bool Gen_Mesh = true;
    bool Ready_Render = false;
    bool DirtyFlag = true;
    std::vector<Vertex> Mesh;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei indexCount = 0;

    Chunk() : blocks(0), width(0), height(0), depth(0), chunkX(0), chunkZ(0) {}

    [[nodiscard]] inline int index(const int x, const int y, const int z) const {
    /*assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    assert(z >= 0 && z < depth);*/

    const int idx = x + z * width + y * width * depth;
    //assert(idx >= 0 && idx < (int)blocks.size());
    return idx;
}


    [[nodiscard]] inline const Block& get(const int x, const int y, const int z) const noexcept {
        return blocks[index(x, y, z)];
    }

    inline void set(const int x, const int y, const int z, const Block& block) {
        blocks.at(index(x, y, z)) = block;
    }

    inline void setID(const int x, const int y, const int z, const uint8_t id) {
        blocks.at(index(x, y, z)).id = id;
    }

    void SendData();

    void RemoveData();
};