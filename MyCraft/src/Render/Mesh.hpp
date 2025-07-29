#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "World/Chunk.hpp"
#include "Camera.hpp"
#include "World/World.hpp"

class Mesh {
    public:

    void GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, int chunkX, int chunkZ, const glm::ivec3 ChunkSize, int RenderDist, const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash> &World);
    void CubeMesh(std::vector<float>& vertices, glm::vec3 w, const Chunk& chunk, glm::ivec3 Local, const glm::ivec3 ChunkSize, const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash> &World);
    bool IsBlockAt(const std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash> &World, int WorldX, int y, int WorldZ, const glm::ivec3 ChunkSize);
};