#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "World/Chunk.hpp"

class Mesh {
    public:
    static void GenerateMesh(const Chunk& chunk, std::vector<Chunk::Vertex>& vertices, int chunkX, int chunkZ, glm::ivec3 ChunkSize, int RenderDist);

    static void MeshZFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, int width, const glm::ivec2& texCoord, int dir);

    static void MeshYFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, int width, const glm::ivec2& texCoord, int dir);

    static void MeshXFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, int width, const glm::ivec2& texCoord, int dir);

    static inline bool IsBlockAt(int WorldX, int y, int WorldZ, const glm::ivec3& ChunkSize);
};