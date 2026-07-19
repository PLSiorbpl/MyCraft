#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

#include "World/World.hpp"

class Fun {
    public:
    static float ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio);

    static size_t calculateWorldMemory(const std::unordered_map<std::pair<int, int>, Chunk, World_Map::PairHash>& World, glm::ivec3 ChunkSize);

    static std::string FormatNumber(uint64_t n);

    static std::string FormatSize(uint64_t n);
};