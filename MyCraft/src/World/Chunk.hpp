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
        int8_t id;
        // Flags: Solid, Transparent, ...
        uint8_t Flags;

        Block(uint8_t id = 0, uint8_t Flags = 0)
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
    bool Gen_Mesh = true;
    bool Ready_Render = false;
    bool DirtyFlag = true;
    std::vector<Vertex> Mesh;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei indexCount = 0;

    Chunk() : width(0), height(0), depth(0), chunkX(0), chunkZ(0), blocks(0) {}

    inline int index(int x, int y, int z) const noexcept {
        return x + z * width + y * width * depth;
    }

    inline const Block& get(int x, int y, int z) const noexcept {
        return blocks[index(x, y, z)];
    }

    void set(int x, int y, int z, const Block& block) {
        blocks.at(index(x, y, z)) = block;
    }

    void setID(int x, int y, int z, uint8_t id) {
        blocks.at(index(x, y, z)).id = id;
    }

    void SendData();

    void RemoveData();
};