#pragma once
#include "World/Chunk.hpp"
#include <vector>
#include <glm/glm.hpp>

class Mesh {
    private:

    public:

    void GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, int chunkX, int chunkZ, glm::ivec3 ChunkSize, int RenderDist);
    void CubeMesh(std::vector<float>& vertices, glm::vec3 w, const Chunk& chunk, glm::ivec3 Local, glm::ivec3 ChunkSize);
};