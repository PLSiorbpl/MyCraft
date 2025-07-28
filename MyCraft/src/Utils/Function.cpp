#include "Function.hpp"

float Fun::ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio) {
    float fovX_rad = glm::radians(fovX_deg);
    float fovY_rad = 2.0f * atan(tan(fovX_rad / 2.0f) / aspectRatio);
    return glm::degrees(fovY_rad);
}

int Fun::calculateWorldMemory(const std::map<std::pair<int,int>, Chunk>& World) {
    int total = 0;
    const int nodeOverhead = sizeof(void*) * 3;

    for (const auto& [key, chunk] : World) {
        total += sizeof(std::pair<const std::pair<int,int>, Chunk>) + nodeOverhead;
        total += chunk.width * chunk.height * chunk.depth * sizeof(Chunk::Block);
    }

    return total;
}
