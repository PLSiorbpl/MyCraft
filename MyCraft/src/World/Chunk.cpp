#include "Chunk.hpp"

std::map<uint8_t, Block> Chunk::BlockDefs = {
    { 0, Block(0, false, false) }, // Air
    { 1, Block(1, false, true)  }, // Stone
    { 2, Block(2, false,  true)  }, // Grass
};