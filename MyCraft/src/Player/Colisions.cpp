#include "Colisions.hpp"

#include "World/World.hpp"

bool colisions::isSolidAround(const glm::vec3 pos, float margin, float height) {
    for (const float dx : {-margin, margin}) {
        for (const float dz : {-margin, margin}) {
            for (const float dy : {0.0f, height / 2.0f, height}) {
                const glm::vec3 offsetPos = pos + glm::vec3(dx, dy, dz);
                if (isSolidAt(offsetPos)) {
                    return true;
                }
            }
        }
    }
    return false;
}
bool colisions::isSolidAt(const glm::vec3 pos) {
    const auto& World = World_Map::World;
    const int blockX = static_cast<int>(floor(pos.x));
    const int blockY = static_cast<int>(floor(pos.y-1.7f));
    const int blockZ = static_cast<int>(floor(pos.z));

    const int chunkX = floor(static_cast<float>(blockX) / Chunk::WIDTH);
    const int chunkZ = floor(static_cast<float>(blockZ) / Chunk::DEPTH);

    const int localX = blockX - chunkX * Chunk::WIDTH;
    const int localZ = blockZ - chunkZ * Chunk::DEPTH;

    if (localX < 0 || localX >= Chunk::WIDTH || localZ < 0 || localZ >= Chunk::DEPTH) {
        return false;
    }
    
    auto it = World.find({chunkX, chunkZ});
    if (it != World.end()) {
        const Chunk& chunk = it->second;

        if (blockY >= 0 && blockY < Chunk::HEIGHT) {
            return chunk.get_state(localX, blockY, localZ)->is_solid;
        }
    }

    return false;
}