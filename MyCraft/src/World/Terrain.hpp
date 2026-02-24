#pragma once
#include "FastNoiseLite.h"
#include <glm/glm.hpp>

#include "Globals.hpp"
#include "World.hpp"

class TerrainGen {
private:
    float baseFreq;
    float baseAmp;
    int octaves;
    float AddedFreq;
    float AddedAmp;
    float BiomeFreq;
    float BiomeMult;
    float BiomeBase;
    float BiomePower;
    FastNoiseLite terrainNoise;
    FastNoiseLite biomeNoise;

    FastNoiseLite Continentalness;
    FastNoiseLite Erosion;
    FastNoiseLite Weidness;
    FastNoiseLite Peaks;
public:
    TerrainGen() {
        terrainNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        biomeNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        terrainNoise.SetSeed(terrain_settings.Seed);
        biomeNoise.SetSeed(terrain_settings.Seed + 420);

        Continentalness.SetSeed(terrain_settings.Seed);
        Erosion.SetSeed(terrain_settings.Seed+67);
        Weidness.SetSeed(terrain_settings.Seed+420);
        Peaks.SetSeed(terrain_settings.Seed+2137);

        Continentalness.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        Erosion.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        Weidness.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
        Peaks.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
    }

    [[nodiscard]] Chunk Generate_Terrain_Chunk(int ChunkX, int ChunkZ, glm::ivec3 ChunkSize) const;
};
