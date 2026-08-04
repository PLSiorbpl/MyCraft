#include "Block.hpp"
#include "glm/glm.hpp"
#include "World.hpp"
#include "Tick/Tick.hpp"

void Lamp::onNeighborChanged(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = 0;
    World_Map::notifyNeighborBlocks(pos, chunk, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (p > 0) return;
        if (auto* b = ch.get_state(x, y, z)) {
            if (b->conductsPower()) {
                World_Map::notifyNeighborBlocks({x, y, z}, cpos, [&p](Chunk& ch_2, int x_2, int y_2, int z_2, const glm::ivec2& cpos_2) {
                    if (p > 0) return;
                    if (const auto b = ch_2.get_state(x_2, y_2, z_2))
                        p = b->getPower({x_2, y_2, z_2}, cpos_2);
                });
                if (p > 0) return;
            }
            p = b->getPower({x, y, z}, cpos);
        }
    });

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);

    lit = p > 0;

    if (lit) {
        uv = {0, 4};
        model = &Models_cache["Lamp_lit"];
    } else {
        model = &Models_cache["Lamp"];
        uv = {7, 0};
    }
}

uint8_t Redstone_dust::getPower(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    return power;
}

void Redstone_dust::onNeighborChanged(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = 0;
    World_Map::notifyNeighborBlocks(pos, chunk, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (const auto b = ch.get_state(x, y, z))
            p = std::max(p, b->getPower({x, y, z}, cpos));
    });

    if (p > 0)
        p = p - 1;
    else
        p = 0;

    if (p == power) return;

    power = p;

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);

    World_Map::notifyNeighborBlocks(pos, chunk, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (const auto b = ch.get_state(x, y, z)) {
            if (b->conductsPower()) {
                World_Map::notifyNeighborBlocks({x, y, z}, cpos, [](Chunk& ch_2, int x_2, int y_2, int z_2, const glm::ivec2& cpos_2) {
                    if (const auto b = ch_2.get_state(x_2, y_2, z_2))
                        Tick::Instant_queue.push({{x_2, y_2, z_2}, cpos_2});
                });
            }
            Tick::Instant_queue.push({{x, y, z}, cpos});
        }
    });
}
