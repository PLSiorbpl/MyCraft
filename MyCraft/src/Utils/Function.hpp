#pragma once
#include <glm/glm.hpp>
#include <map>
#include "World/Chunk.hpp"

class Fun {
    public:
    
    float ConvertHorizontalFovToVertical(float fovX_deg, float aspectRatio) const;
    int calculateWorldMemory(const std::map<std::pair<int, int>, Chunk>& World);
};