#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include "World/Chunk.hpp"
#include "World/World.hpp"

class Mesh {
    public:
    static void GenerateMesh(const Chunk& chunk, std::vector<Chunk::Vertex>& vertices, const int chunkX, const int chunkZ, const glm::ivec3 ChunkSize, const int RenderDist);

    static void MeshZFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir);

    static void MeshYFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir);

    static void MeshXFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir);

    static inline bool IsBlockAt(int WorldX, int y, int WorldZ, const glm::ivec3& ChunkSize);
};