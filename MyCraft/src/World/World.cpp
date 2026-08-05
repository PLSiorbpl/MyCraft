#include "World/World.hpp"

#include <algorithm>

#include "Tick/Tick.hpp"

namespace World_Map {
    std::unordered_map<std::pair<int, int>, Chunk, PairHash> World;
    std::vector<Render_Info> Render_List;
    std::vector<Chunk*> Mesh_Queue;

    void Set_Dirty(const int chunkx, const int chunkz) {
        const auto chunk = find_chunk(chunkx, chunkz);
        if (chunk) {
            chunk->DirtyFlag = true;
            chunk->has_mesh = false;
            for (auto& info : Render_List) {
                if (info.chunkX == chunkx && info.chunkZ == chunkz)
                    info.Delete = 1;
            }
            if(std::find(Mesh_Queue.begin(), Mesh_Queue.end(), chunk) == Mesh_Queue.end()) {
                Mesh_Queue.push_back(chunk);
            }
        }
    }

    void Set_Neighbors_Dirty(const int localx, const int localz, const int chunkx, const int chunkz) {
        if (localx == 0) Set_Dirty(chunkx-1, chunkz);
        if (localx == Chunk::WIDTH-1) Set_Dirty(chunkx+1, chunkz);
        if (localz == 0) Set_Dirty(chunkx, chunkz-1);
        if (localz == Chunk::DEPTH-1) Set_Dirty(chunkx, chunkz+1);
    }

    void notifyNeighborBlocks(const glm::ivec3 &pos, const glm::ivec2 &chunkPos) {
        forEachNeighbor(pos, chunkPos, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
            if (ch.get_state(x, y, z))
                Tick::Instant_queue.push({{x, y, z}, cpos});
        });
    }

    void notifyNeighborBlocksConduct(const glm::ivec3 &pos, const glm::ivec2 &chunkPos) {
        forEachNeighbor(pos, chunkPos, [](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
        if (const auto b = ch.get_state(x, y, z)) {
            if (b->conductsPower()) {
                forEachNeighbor({x, y, z}, cpos, [](Chunk& ch_2, int x_2, int y_2, int z_2, const glm::ivec2& cpos_2) {
                    if (ch_2.get_state(x_2, y_2, z_2))
                        Tick::Instant_queue.push({{x_2, y_2, z_2}, cpos_2});
                });
            }
            Tick::Instant_queue.push({{x, y, z}, cpos});
        }
    });
    }

    uint8_t getMAX_Neighbor_Power(const glm::ivec3 &pos, const glm::ivec2 &chunkPos) {
        uint8_t p = 0;
        forEachNeighbor(pos, chunkPos, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
            if (const auto b = ch.get_state(x, y, z))
                p = std::max(p, b->getPower({x, y, z}, cpos));
        });
        return p;
    }

    uint8_t getMAX_Neighbor_Conduct_Power(const glm::ivec3 &pos, const glm::ivec2 &chunkPos) {
        uint8_t p = 0;
        forEachNeighbor(pos, chunkPos, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
            if (const auto b = ch.get_state(x, y, z)) {
                if (b->conductsPower()) {
                    p = std::max(p, getMAX_Neighbor_Power({x, y, z}, cpos));
                }
                p = std::max(p, b->getPower({x, y, z}, cpos));
            }
        });
        return p;
    }

    uint8_t getANY_Neighbor_Power(const glm::ivec3 &pos, const glm::ivec2 &chunkPos) {
        uint8_t p = 0;
        forEachNeighbor(pos, chunkPos, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
            if (p > 0) return;
            if (const auto b = ch.get_state(x, y, z))
                p = b->getPower({x, y, z}, cpos);
        });
        return p;
    }

    uint8_t getANY_Neighbor_Conduct_Power(const glm::ivec3 &pos, const glm::ivec2 &chunkPos) {
        uint8_t p = 0;
        forEachNeighbor(pos, chunkPos, [&p](Chunk& ch, int x, int y, int z, const glm::ivec2& cpos) {
            if (p > 0) return;
            if (const auto b = ch.get_state(x, y, z)) {
                if (b->conductsPower()) {
                    p = getANY_Neighbor_Power({x, y, z}, cpos);
                }
                if (p > 0) return;
                p = b->getPower({x, y, z}, cpos);
            }
        });
        return p;
    }
}
