#pragma once
#include "Block.hpp"
#include <vector>
#include <map>

class Chunk {
public:
    std::vector<Block> blocks;
    int width, height, depth;
    int chunkX, chunkZ;
    static std::map<uint8_t, Block> BlockDefs;

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
};