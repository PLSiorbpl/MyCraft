#pragma once
#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>

#include "Chunk.hpp"

class World_Map {
    public:
    struct PairHash {
        std::size_t operator()(const std::pair<int, int>& p) const {
            uint64_t z = (static_cast<uint64_t>(static_cast<uint32_t>(p.first)) << 32) | static_cast<uint32_t>(p.second);

            // SplitMix64 scrambling
            z += 0x9e3779b97f4a7c15;
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
            z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
            return z ^ (z >> 31);
        }
    };

    struct Render_Info {
        int chunkX, chunkZ;
        GLuint vao;
        GLuint vbo;
        GLsizei indexCount;
        size_t Mesh_Size;
        size_t Capacity;
        uint64_t Triangles;
        int Delete;
    };

    static std::unordered_map<std::pair<int, int>, Chunk, PairHash> World;
    static std::vector<Render_Info> Render_List;
    static std::vector<Chunk*> Mesh_Queue;

    static Chunk *find_chunk(const int chunkx, const int chunkz) {
        const auto c = World.find({chunkx, chunkz});
        if (c != World.end())
            return &c->second;
        return nullptr;
    }

    static void Set_Dirty(int chunkx, int chunkz);
    static void Set_Neighbors_Dirty(int localx, int localz, int chunkx, int chunkz);

    template<typename Func>
    static void notifyNeighborBlocks(const glm::ivec3& pos, const glm::ivec2& chunkPos, Func&& fn) {
        auto* c = find_chunk(chunkPos.x, chunkPos.y);
        if (!c) return;

        auto call = [&](Chunk* ch, int x, int y, int z, const glm::ivec2& cpos) {
            if (!ch) return;

            if (ch->get(x, y, z).id == block_type::Air)
                return;

            if (ch->get(x, y, z).state == 0) {
                ch->create_state(x, y, z);
            }

            fn(*ch, x, y, z, cpos);
        };

        // X-
        if (pos.x - 1 < 0) {
            auto* n = find_chunk(chunkPos.x - 1, chunkPos.y);
            if (n) call(n, Chunk::WIDTH - 1, pos.y, pos.z, {chunkPos.x - 1, chunkPos.y});
        } else {
            call(c, pos.x - 1, pos.y, pos.z, chunkPos);
        }

        // X+
        if (pos.x + 1 >= Chunk::WIDTH) {
            auto* n = find_chunk(chunkPos.x + 1, chunkPos.y);
            if (n) call(n, 0, pos.y, pos.z, {chunkPos.x + 1, chunkPos.y});
        } else {
            call(c, pos.x + 1, pos.y, pos.z, chunkPos);
        }

        // Z-
        if (pos.z - 1 < 0) {
            auto* n = find_chunk(chunkPos.x, chunkPos.y - 1);
            if (n) call(n, pos.x, pos.y, Chunk::DEPTH - 1, {chunkPos.x, chunkPos.y - 1});
        } else {
            call(c, pos.x, pos.y, pos.z - 1, chunkPos);
        }

        // Z+
        if (pos.z + 1 >= Chunk::DEPTH) {
            auto* n = find_chunk(chunkPos.x, chunkPos.y + 1);
            if (n) call(n, pos.x, pos.y, 0, {chunkPos.x, chunkPos.y + 1});
        } else {
            call(c, pos.x, pos.y, pos.z + 1, chunkPos);
        }

        // Y+
        if (pos.y + 1 < Chunk::HEIGHT) {
            call(c, pos.x, pos.y + 1, pos.z, chunkPos);
        }

        // Y-
        if (pos.y - 1 >= 0) {
            call(c, pos.x, pos.y - 1, pos.z, chunkPos);
        }
    }
};