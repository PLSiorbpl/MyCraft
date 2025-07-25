#pragma once
#include <glm/glm.hpp>
#include <map>
#include "World/Chunk.hpp"

class colisions {
    public:
    bool isSolidAround(glm::vec3 pos, const std::map<std::pair<int, int>, Chunk>& World, glm::ivec3 ChunkSize, float margin = 0.25f, float height = 1.8f);
    bool isSolidAt(glm::vec3 pos, const std::map<std::pair<int, int>, Chunk>& World, glm::ivec3 ChunkSize);
};