#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <array>

#include "World/World.hpp"
#include "Render/Camera.hpp"
#include "World/Chunk.hpp"
#include "Colisions.hpp"

class Terrain_Action {
    private:
        colisions Colision;
    public:
        void RayCastBlock(camera &Camera, const glm::ivec3 ChunkSize, bool Action, int block, float MaxDistance = 4.8f, float StepSize = 0.05f);
};