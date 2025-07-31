#include "Function.hpp"

float Fun::ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio) const {
    return glm::degrees(2.0f * atan(tan(glm::radians(fovX_deg) / 2.0f) / aspectRatio));
}

int Fun::calculateWorldMemory(const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash>& World) {
    int total = 0;
    const int nodeOverhead = sizeof(void*) * 3;

    for (const auto& [key, chunk] : World) {
        total += sizeof(std::pair<const std::pair<int,int>, Chunk>) + nodeOverhead;
        total += chunk.width * chunk.height * chunk.depth * sizeof(Chunk::Block);
    }

    return total;
}
