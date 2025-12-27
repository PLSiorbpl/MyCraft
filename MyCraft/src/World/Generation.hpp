#pragma once
#include <glm/glm.hpp>

#include "Terrain.hpp"
# include "Utils/Globals.hpp"

class ChunkGeneration {
    private:
    int chunkX, chunkZ;
    int dx, dz, dist;
    TerrainGen Terrain;

    public:
    ChunkGeneration(int seed, float basefreq, float baseamp, int oct, float addfreq, float addamp, float biomefreq, float biomemult, float biomebase, int biomepower);

    void GenerateChunks(const glm::ivec3 ChunkSize);

    static void RemoveChunks();
};