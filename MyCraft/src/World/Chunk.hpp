#pragma once
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
        bool transparent;
        bool solid;
        uint8_t light;

        Block(uint8_t id = 0, bool transparent = false, bool solid = false, uint8_t light = 0)
            : id(id), transparent(transparent), solid(solid), light(light) {}
    };

    // World Stuff
    std::vector<Block> blocks;
    int width, height, depth;
    int chunkX, chunkZ;
    static const std::map<uint8_t, Block> BlockDefs;

    // Mesh Stuff
    size_t Alloc = 0;
    bool Gen_Mesh = true;
    std::vector<float> Mesh;
    bool DirtyFlag = true;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei indexCount = 0;

    Chunk() : width(0), height(0), depth(0), chunkX(0), chunkZ(0), blocks(0) {}

    int index(int x, int y, int z) const {
        return x + z * width + y * width * depth;
    }

    const Block& get(int x, int y, int z) const {
        return blocks.at(index(x, y, z));
    }

    void set(int x, int y, int z, const Block& block) {
        blocks.at(index(x, y, z)) = block;
    }

    void setID(int x, int y, int z, uint8_t id) {
        blocks.at(index(x, y, z)).id = id;
    }

    void Allocate();

    void SendData();

    void RemoveData();
};