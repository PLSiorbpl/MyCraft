#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include "World/Chunk.hpp"
#include "World/World.hpp"
#include <iomanip>

class Fun {
    public:
    
    float ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio) const;
    size_t calculateWorldMemory(const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash>& World, glm::ivec3 ChunkSize);
    std::string FormatNumber(uint64_t n);
    std::string FormatSize(uint64_t n);
};