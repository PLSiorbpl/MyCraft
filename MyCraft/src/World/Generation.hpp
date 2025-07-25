#pragma once
#include <map>
#include "Chunk.hpp"
#include "Render/Camera.hpp"
#include "Terrain.hpp"
#include <glm/glm.hpp>
#include <iostream>

class ChunkGeneration {
    private:
    int chunkX, chunkZ;
    int dx, dz, dist;
    TerrainGen Terrain;

    public:
    ChunkGeneration(int seed);

    void GenerateChunks(camera &Camera, std::map<std::pair<int, int>, Chunk>& World, glm::ivec3 ChunkSize);
    void RemoveChunks(camera &Camera, std::map<std::pair<int, int>, Chunk>& World);
};