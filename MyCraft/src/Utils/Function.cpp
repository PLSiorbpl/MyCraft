#include "Function.hpp"

float Fun::ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio) const {
    return glm::degrees(2.0f * atan(tan(glm::radians(fovX_deg) / 2.0f) / aspectRatio));
}

size_t Fun::calculateWorldMemory(const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash>& World, glm::ivec3 ChunkSize) {
    size_t total = 0;
    const int nodeOverhead = sizeof(void*) * 3;
    
    total = World.size() * ChunkSize.x * ChunkSize.y * ChunkSize.z * sizeof(Chunk::Block);
    total += (sizeof(std::pair<const std::pair<int,int>, Chunk>) + nodeOverhead) * World.size();


    return total;
}
