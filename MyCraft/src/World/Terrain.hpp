#pragma once
#include <map>
#include "Chunk.hpp"
#include "FastNoiseLite.h"
#include <glm/glm.hpp>

class TerrainGen {
private:
    int Seed;
    float baseFreq = 0.5f;
    float baseAmp = 0.2f;
    int octaves = 4;
    FastNoiseLite terrainNoise;
    FastNoiseLite biomeNoise;

public:
    TerrainGen(int seed);
    void Generate_Terrain_Chunk(int ChunkX, int ChunkZ, std::map<std::pair<int, int>, Chunk>& World, glm::ivec3 ChunkSize);
};
