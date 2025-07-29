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
    ChunkGeneration(int seed, float basefreq, float baseamp, int oct, float addfreq, float addamp, float biomefreq, float biomemult, float biomebase);

    void GenerateChunks(const camera &Camera, std::map<std::pair<int, int>, Chunk>& World, const glm::ivec3 ChunkSize);
    void RemoveChunks(const camera &Camera, std::map<std::pair<int, int>, Chunk>& World);
};