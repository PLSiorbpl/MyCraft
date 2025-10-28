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

    void GenerateMesh(const Chunk& chunk, std::vector<Chunk::Vertex>& vertices, const int chunkX, const int chunkZ, const glm::ivec3 ChunkSize, const int RenderDist);
    void MeshZFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir);
    void MeshYFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir);
    void MeshXFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir);
    inline bool IsBlockAt(int WorldX, int y, int WorldZ, const glm::ivec3& ChunkSize);
};