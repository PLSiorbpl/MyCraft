#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include "World/Chunk.hpp"
#include "Camera.hpp"
#include "World/World.hpp"

class Mesh {
    public:

    void GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, int chunkX, int chunkZ, const glm::ivec3 ChunkSize, int RenderDist);
    void CubeMesh(std::vector<float>& vertices, glm::vec3 w, const Chunk& chunk, glm::ivec3 Local, const glm::ivec3 ChunkSized);
    bool IsBlockAt(int WorldX, int y, int WorldZ, const glm::ivec3 ChunkSize);
};