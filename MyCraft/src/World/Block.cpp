#include "Block.hpp"
#include "glm/glm.hpp"
#include "World.hpp"
#include <cstdio>

void Iron::onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    World_Map::notifyNeighborBlocks(pos, chunk, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        ch.get_state(x, y, z)->onActivate({x, y, z}, cpos);
    });
}

void Iron::onRemove(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    World_Map::notifyNeighborBlocks(pos, chunk, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        ch.get_state(x, y, z)->onActivate({x, y, z}, cpos);
    });
}
