#pragma once
#include <queue>

#include "Movement.hpp"
#include "SelectionBox.hpp"

namespace Tick {
    struct Tick_Update {
        glm::ivec3 pos;
        glm::ivec2 chunk;
    };

    void Tick(Movement &movement, Selection &Sel);

    extern std::queue<Tick_Update> Instant_queue;
}
