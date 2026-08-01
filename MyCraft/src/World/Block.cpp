#include "Block.hpp"
#include "glm/glm.hpp"
#include "World.hpp"

void Iron::onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    powered = true;
    World_Map::notifyNeighborBlocks(pos, chunk, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (auto* b = ch.get_state(x, y, z))
            b->onNeighborChanged({x, y, z}, cpos);
    });
}

void Iron::onRemove(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    powered = false;
    World_Map::notifyNeighborBlocks(pos, chunk, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (auto* b = ch.get_state(x, y, z))
            b->onNeighborChanged({x, y, z}, cpos);
    });
}

void Lamp::onNeighborChanged(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    bool p = false;
    World_Map::notifyNeighborBlocks(pos, chunk, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (p) return;
        if (auto* b = ch.get_state(x, y, z))
            p = b->isPowered({x, y, z}, cpos, true);
    });

    lit = p;

    if (lit) {
        uv = {0, 4};
        model = &Models_cache["Lamp_lit"];
    } else {
        model = &Models_cache["Lamp"];
        uv = {7, 0};
    }
}

void Redstone_dust::onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    onNeighborChanged(pos, chunk);
}

void Redstone_dust::onRemove(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    onNeighborChanged(pos, chunk);
}

bool Redstone_dust::isPowered(const glm::ivec3 &pos, const glm::ivec2 &chunk, const bool is_source) {
    if (updated) return false;
    updated = true;

    bool p = false;
    World_Map::notifyNeighborBlocks(pos, chunk, [&p, is_source](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (p) return;
        if (auto* b = ch.get_state(x, y, z))
            p = b->isPowered({x, y, z}, cpos, is_source);
    });

    updated = false;
    return p;
}

void Redstone_dust::onNeighborChanged(const glm::ivec3 &pos, const glm::ivec2 &chunk) {
    if (updated) return;
    updated = true;

    bool p = false;
    World_Map::notifyNeighborBlocks(pos, chunk, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (p) return;
        if (auto *b = ch.get_state(x, y, z))
            p = b->isPowered({x, y, z}, cpos, true);
    });

    const bool changed = (p != powered);
    powered = p;
    model = powered ? &Models_cache["Redstone_dust_powered"] : &Models_cache["Redstone_dust"];

    updated = false;

    if (changed) {
        World_Map::notifyNeighborBlocks(pos, chunk, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
            if (auto* b = ch.get_state(x, y, z))
                b->onNeighborChanged({x, y, z}, cpos);
        });
    }
}
