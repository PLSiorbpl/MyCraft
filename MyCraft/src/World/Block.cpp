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
        model = Models_cache["Lamp_lit"].get(Direction::North);
    } else {
        model = Models_cache["Lamp"].get(Direction::North);
        uv = {7, 0};
    }
}

void Redstone_dust::onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = World_Map::getMAX_Neighbor_Conduct_Power(pos, chunk);

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

    if (powered) model = Models_cache["Repeater On"].get(direction_);
    else model = Models_cache["Repeater Off"].get(direction_);

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);

    World_Map::notifyNeighborBlocksConduct(pos, chunk);
}

void Redstone_Torch::onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    uint8_t p = World_Map::getANY_Conduct_Power(pos, chunk, direction_);
    p = p > 0;

    if (p == powered) return;

    if (!scheduled) {
        scheduled = true;
        Tick::Tick_queue.push_back({pos, chunk, delay});
    }
}

void Redstone_Torch::onTickUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    scheduled = false;
    uint8_t p = World_Map::getANY_Conduct_Power(pos, chunk, direction_);
    p = p > 0;

    if (p == powered) return;
    powered = p;

    if (powered) model = Models_cache["Redstone Torch On"].get(direction_);
    else model = Models_cache["Redstone Torch Off"].get(direction_);

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);

    World_Map::notifyNeighborBlocksConduct(pos, chunk);
}

void Comparator::onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    const int8_t In = World_Map::getMAX_Conduct_Power(pos, chunk, Opposite(direction_));
    const int8_t right = World_Map::get_Power(pos, chunk, Right(direction_));
    const int8_t left = World_Map::get_Power(pos, chunk, Left(direction_));

    if (!subtract) {
        uint8_t Out = In;
        Out = In < right ? 0 : Out;
        Out = In < left ? 0 : Out;
        if (Out == power) return;
    } else {
        const uint8_t Out = std::max(right > left ? In - right : In - left, 0);
        if (Out == power) return;
    }

    if (!scheduled) {
        scheduled = true;
        Tick::Tick_queue.push_back({pos, chunk, delay});
    }
}

void Comparator::onTickUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    scheduled = false;
    const uint8_t In = World_Map::getMAX_Conduct_Power(pos, chunk, Opposite(direction_));
    const uint8_t right = World_Map::get_Power(pos, chunk, Right(direction_));
    const uint8_t left = World_Map::get_Power(pos, chunk, Left(direction_));

    if (!subtract) {
        uint8_t Out = In;
        Out = In < right ? 0 : Out;
        Out = In < left ? 0 : Out;

        if (Out == power) return;
        power = Out;
    } else {
        const uint8_t Out = std::max(right > left ? In - right : In - left, 0);
        if (Out == power) return;
        power = Out;
    }

    onPlace(pos, chunk, direction_);

    World_Map::notifyNeighborBlocksConduct(pos, chunk);
}

void Repeater::onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk, Direction dir) {
    direction_ = dir;
    if (powered) model = Models_cache["Repeater On"].get(direction_);
    else model = Models_cache["Repeater Off"].get(direction_);

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);
}

void Redstone_Torch::onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk, Direction dir) {
    direction_ = dir;
    if (powered) model = Models_cache["Redstone Torch On"].get(direction_);
    else model = Models_cache["Redstone Torch Off"].get(direction_);

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);
}

void Comparator::onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk, Direction dir) {
    direction_ = dir;
    if (!subtract) {
        if (power > 0) model = Models_cache["Comparator On"].get(direction_);
        else model = Models_cache["Comparator Off"].get(direction_);
    } else {
        if (power > 0) model = Models_cache["Comparator On Sub"].get(direction_);
        else model = Models_cache["Comparator Off Sub"].get(direction_);
    }

    World_Map::Set_Dirty(chunk.x, chunk.y);
    World_Map::Set_Neighbors_Dirty(pos.x, pos.z, chunk.x, chunk.y);
}
