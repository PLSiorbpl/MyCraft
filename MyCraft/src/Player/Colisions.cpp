#include "Colisions.hpp"

bool colisions::isSolidAround(glm::vec3 pos, const glm::ivec3 ChunkSize, float margin, float height) {
    for (const float dx : {-margin, margin}) {
        for (const float dz : {-margin, margin}) {
            for (const float dy : {0.0f, height / 2.0f, height}) {
                const glm::vec3 offsetPos = pos + glm::vec3(dx, dy, dz);
                if (isSolidAt(offsetPos, ChunkSize)) {
                    return true;
                }
            }
        }
    }
    return false;
}
bool colisions::isSolidAt(glm::vec3 pos, const glm::ivec3 ChunkSize) {
    const auto& World = World_Map::World;
    const int blockX = static_cast<int>(floor(pos.x));
    const int blockY = static_cast<int>(floor(pos.y-1.7f));
    const int blockZ = static_cast<int>(floor(pos.z));

    const int chunkX = floor(static_cast<float>(blockX) / ChunkSize.x);
    const int chunkZ = floor(static_cast<float>(blockZ) / ChunkSize.z);

    const int localX = blockX - chunkX * ChunkSize.x;
    const int localZ = blockZ - chunkZ * ChunkSize.z;

    if (localX < 0 || localX >= ChunkSize.x || localZ < 0 || localZ >= ChunkSize.z) {
        return false;
    }
    
    auto it = World.find({chunkX, chunkZ});
    if (it != World.end()) {
        const Chunk& chunk = it->second;

        if (blockY >= 0 && blockY < ChunkSize.y) {
            return chunk.get(localX, blockY, localZ).Flags & 0b1000'0000;
        }
    }

    return false;
}