#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include "World/Chunk.hpp"
#include "World/World.hpp"

class colisions {
    public:
    bool isSolidAround(glm::vec3 pos, const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash> &World, const glm::ivec3 ChunkSize, float margin = 0.25f, float height = 1.8f);
    bool isSolidAt(glm::vec3 pos, const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash> &World, const glm::ivec3 ChunkSize);
};