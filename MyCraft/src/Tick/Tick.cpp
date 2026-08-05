#include "Tick.hpp"
#include "World/World.hpp"

namespace Tick {
    std::queue<Instant_Update> Instant_queue;
    std::vector<Tick_Update> Tick_queue;

    void Tick(Movement &movement, Selection &Sel) {
        movement.Init(Sel);

        while (!Instant_queue.empty()) {
            const auto [pos, chunk] = Instant_queue.front();
            Instant_queue.pop();

            auto *ch = World_Map::find_chunk(chunk.x, chunk.y);
            if (!ch) continue;

            const auto b = ch->get_state(pos.x, pos.y, pos.z);
            b->onInstantUpdate(pos, chunk);
        }

        for (auto &[pos, chunk, delay, finished] : Tick_queue) {
            if (delay-- > 0) continue;

            auto *ch = World_Map::find_chunk(chunk.x, chunk.y);
            if (!ch) continue;

            const auto b = ch->get_state(pos.x, pos.y, pos.z);
            b->onTickUpdate(pos, chunk);

            finished = true;
        }

        Tick_queue.erase(std::remove_if(Tick_queue.begin(), Tick_queue.end(), [](const auto& e) { return e.finished; }),Tick_queue.end());
    }
}
