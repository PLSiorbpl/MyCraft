#pragma once
#include "Chunk.hpp"
#include "FastNoiseLite.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <unordered_map>
#include "World.hpp"

class TerrainGen {
private:
    int Seed;
    float baseFreq = 0.5f;
    float baseAmp = 0.2f;
    int octaves = 4;
    float AddedFreq = 2.0f;
    float AddedAmp = 0.2f;
    float BiomeFreq = 0.03f;
    float BiomeMult = 0.5f;
    float BiomeBase = 1.5f;
    int BiomePower = 4;
    FastNoiseLite terrainNoise;
    FastNoiseLite biomeNoise;

public:
    TerrainGen(int seed, float basefreq, float baseamp, int oct, float addfreq, float addamp, float biomefreq, float biomemult, float biomebase, int biomepower)
    : Seed(seed), baseFreq(basefreq), baseAmp(baseamp), octaves(oct),
      AddedFreq(addfreq), AddedAmp(addamp), BiomeFreq(biomefreq),
      BiomeMult(biomemult), BiomeBase(biomebase), BiomePower(biomepower)
    {
        terrainNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        biomeNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        terrainNoise.SetSeed(seed);
        biomeNoise.SetSeed(seed + 420);
    }

    void Generate_Terrain_Chunk(const int ChunkX, const int ChunkZ, const glm::ivec3 ChunkSize);
};
