#pragma once
#include <queue>
#include <vector>

#include "Player/Movement.hpp"
#include "Render/SelectionBox.hpp"

namespace Tick {
    struct Instant_Update {
        glm::ivec3 pos;
        glm::ivec2 chunk;
    };

    struct Tick_Update {
        glm::ivec3 pos;
        glm::ivec2 chunk;
        int16_t delay;
        bool finished = false;
    };

    void Tick(Movement &movement, Selection &Sel);

    extern std::queue<Instant_Update> Instant_queue;
    extern std::vector<Tick_Update> Tick_queue;
}
