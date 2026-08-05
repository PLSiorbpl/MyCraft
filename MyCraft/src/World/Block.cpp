#include "Block.hpp"
#include "glm/glm.hpp"
#include "World.hpp"
#include "Tick/Tick.hpp"

void Lamp::onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = World_Map::getANY_Neighbor_Conduct_Power(pos, chunk);
    p = p > 0;

    if (lit != p) {
        World_Map::Set_Dirty(chunk.x, chunk.y);
        World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);
    }

    lit = p;

    if (lit) {
        uv = {0, 4};
        model = &Models_cache["Lamp_lit"];
    } else {
        model = &Models_cache["Lamp"];
        uv = {7, 0};
    }
}

void Redstone_dust::onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = World_Map::getMAX_Neighbor_Power(pos, chunk);

    if (p > 0)
        p = p - 1;
    else
        p = 0;

    if (p == power) return;

    power = p;

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);

    World_Map::notifyNeighborBlocksConduct(pos, chunk);
}

void Repeater::onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = World_Map::getANY_Conduct_Power(pos, chunk, Opposite(direction_));
    p = p > 0;

    if (p == powered) return;

    if (!scheduled) {
        scheduled = true;
        Tick::Tick_queue.push_back({pos, chunk, delay});
    }
}

void Repeater::onTickUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    scheduled = false;
    uint8_t p = World_Map::getANY_Conduct_Power(pos, chunk, Opposite(direction_));
    p = p > 0;

    if (p == powered) return;
    powered = p;

    if (powered) model = &Models_cache["Repeater On"];
    else model = &Models_cache["Repeater Off"];

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);

    World_Map::notifyNeighborBlocksConduct(pos, chunk);
}

void Redstone_Torch::onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = World_Map::getANY_Conduct_Power(pos, chunk, Opposite(direction_));
    p = p > 0;

    if (p == powered) return;

    if (!scheduled) {
        scheduled = true;
        Tick::Tick_queue.push_back({pos, chunk, delay});
    }
}

void Redstone_Torch::onTickUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    scheduled = false;
    uint8_t p = World_Map::getANY_Conduct_Power(pos, chunk, Opposite(direction_));
    p = p > 0;

    if (p == powered) return;
    powered = p;

    if (powered) model = &Models_cache["Redstone Torch On"];
    else model = &Models_cache["Redstone Torch Off"];

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);

    World_Map::notifyNeighborBlocksConduct(pos, chunk);
}
