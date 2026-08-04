#include "Tick.hpp"
#include "World/World.hpp"

namespace Tick {
    std::queue<Tick_Update> Instant_queue;

    void Tick(Movement &movement, Selection &Sel) {
        movement.Init(Sel);

        while (!Instant_queue.empty()) {
            const auto [pos, chunk] = Instant_queue.front();
            Instant_queue.pop();

            auto *ch = World_Map::find_chunk(chunk.x, chunk.y);
            if (!ch) continue;

            const auto b = ch->get_state(pos.x, pos.y, pos.z);
            b->onNeighborChanged(pos, chunk);
        }
    }
}
