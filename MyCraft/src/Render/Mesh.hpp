#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "World/Chunk.hpp"
#include "Camera.hpp"

class Mesh {
    public:

    void GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, int chunkX, int chunkZ, glm::ivec3 ChunkSize, int RenderDist, std::map<std::pair<int, int>, Chunk> &World);
    void CubeMesh(std::vector<float>& vertices, glm::vec3 w, const Chunk& chunk, glm::ivec3 Local, glm::ivec3 ChunkSize, std::map<std::pair<int, int>, Chunk> &World);
    bool IsBlockAt(std::map<std::pair<int, int>, Chunk> &World, int WorldX, int y, int WorldZ, glm::ivec3 ChunkSize);
};