#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "World/Chunk.hpp"
#include "Camera.hpp"

class Mesh {
    public:

    void GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, int chunkX, int chunkZ, const glm::ivec3 ChunkSize, int RenderDist, const std::map<std::pair<int, int>, Chunk> &World);
    void CubeMesh(std::vector<float>& vertices, glm::vec3 w, const Chunk& chunk, glm::ivec3 Local, const glm::ivec3 ChunkSize, const std::map<std::pair<int, int>, Chunk> &World);
    bool IsBlockAt(const std::map<std::pair<int, int>, Chunk> &World, int WorldX, int y, int WorldZ, const glm::ivec3 ChunkSize);
};