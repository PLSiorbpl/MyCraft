#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <array>
#include <algorithm>

#include "World/World.hpp"
#include "Render/Camera.hpp"
#include "World/Chunk.hpp"
#include "Colisions.hpp"
#include "Render/SelectionBox.hpp"

class Terrain_Action {
    private:
        colisions Colision;
    public:
        void RayCastBlock(camera &Camera, const glm::ivec3 ChunkSize, int Action, int block, Selection& Sel, bool &ChunkUpdated, float MaxDistance = 6.0f, float StepSize = 0.05f);
};