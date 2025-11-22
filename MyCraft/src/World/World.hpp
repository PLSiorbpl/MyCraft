#pragma once
#include <unordered_map>
#include <memory>
#include <vector>
#include "Chunk.hpp"

class World_Map {
    public:
    struct pair_hash {
        std::size_t operator()(const std::pair<int, int>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
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

    static std::unordered_map<std::pair<int, int>, Chunk, pair_hash> World;
    static std::vector<Render_Info> Render_List;
    static std::vector<Chunk*> Mesh_Queue;
};