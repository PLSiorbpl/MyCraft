#pragma once
#include <glm/glm.hpp>
#include <iostream>

#include "World/World.hpp"
#include "Render/Camera.hpp"
#include "World/Chunk.hpp"

class Terrain_Action {
    public:
    
    void RayCastBlock(const camera &Camera, const glm::ivec3 ChunkSize, bool Action, int block, float MaxDistance = 4.0f, float StepSize = 0.1f);
};