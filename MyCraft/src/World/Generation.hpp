#pragma once
#include "Chunk.hpp"
#include "Render/Camera.hpp"
#include "Terrain.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include "World.hpp"

class ChunkGeneration {
    private:
    int chunkX, chunkZ;
    int dx, dz, dist;
    TerrainGen Terrain;

    public:
    ChunkGeneration(int seed, float basefreq, float baseamp, int oct, float addfreq, float addamp, float biomefreq, float biomemult, float biomebase, int biomepower);

    void GenerateChunks(const camera &Camera, const glm::ivec3 ChunkSize);
    void RemoveChunks(const camera &Camera);
};