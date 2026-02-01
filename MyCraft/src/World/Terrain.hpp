#pragma once
#include "FastNoiseLite.h"
#include <glm/glm.hpp>

#include "Globals.hpp"
#include "World.hpp"

class TerrainGen {
private:
    float baseFreq = 0.5f;
    float baseAmp = 0.2f;
    int octaves = 4;
    float AddedFreq = 2.0f;
    float AddedAmp = 0.2f;
    float BiomeFreq = 0.03f;
    float BiomeMult = 0.5f;
    float BiomeBase = 1.5f;
    float BiomePower = 4;
    FastNoiseLite terrainNoise{0};
    FastNoiseLite biomeNoise;

public:
    TerrainGen() {
        terrainNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        biomeNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        terrainNoise.SetSeed(terrain_settings.Seed);
        biomeNoise.SetSeed(terrain_settings.Seed + 420);
    }

    [[nodiscard]] Chunk Generate_Terrain_Chunk(int ChunkX, int ChunkZ, glm::ivec3 ChunkSize) const;
};
