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

std::string Fun::FormatNumber(uint64_t n) {
    double value = static_cast<double>(n);
    char suffix = '\0';

    if (n >= 1'000'000) { // Milion
        value /= 1'000'000;
        suffix = 'M';
    } else if (n >= 1'000) { // Thousnd
        value /= 1'000;
        suffix = 'K';
    }

    std::ostringstream oss;
    if (suffix) {
        oss << std::fixed << std::setprecision(1) << value << suffix;
    } else {
        oss << n;
    }

    return oss.str();
}

std::string Fun::FormatSize(const uint64_t n) {
    double value = static_cast<double>(n);
    std::string suffix = "B";

    if (n >= 1ULL << 30) {
        value /= 1ULL << 30;
        suffix = "GB";
    } else if (n >= 1ULL << 20) {
        value /= 1ULL << 20;
        suffix = "MB";
    } else if (n >= 1ULL << 10) {
        value /= 1ULL << 10;
        suffix = "KB";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value << suffix;

    return oss.str();
}
