#include "Block.hpp"

Block::Block(uint8_t id, bool transparent, bool solid, uint8_t light)
    : id(id), transparent(transparent), solid(solid), light(light) {}