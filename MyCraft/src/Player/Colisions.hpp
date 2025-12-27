#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include "World/Chunk.hpp"
#include "World/World.hpp"

class colisions {
    public:
    static bool isSolidAround(glm::vec3 pos, const glm::ivec3 ChunkSize, float margin = 0.25f, float height = 1.8f);

    static bool isSolidAt(glm::vec3 pos, const glm::ivec3 ChunkSize);
};