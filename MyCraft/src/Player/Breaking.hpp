#pragma once

#include "Render/Camera.hpp"
#include "Colisions.hpp"
#include "Render/SelectionBox.hpp"

class Terrain_Action {
    private:
        colisions Colision;
    public:
        void RayCastBlock(camera &Camera, int Action, int block, Selection& Sel, float MaxDistance = 6.0f, float StepSize = 0.05f);
};