#pragma once
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
};