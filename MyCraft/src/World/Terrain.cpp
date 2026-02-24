#include "Terrain.hpp"
#include "World.hpp"
#include "Utils/Globals.hpp"

inline float lerp(const float a, const float b, const float t) {
    return a + (b - a) * t;
}

Chunk TerrainGen::Generate_Terrain_Chunk(const int ChunkX, const int ChunkZ, const glm::ivec3 ChunkSize) const {
    Chunk chunk;
    chunk.width = ChunkSize.x;
    chunk.height = ChunkSize.y;
    chunk.depth = ChunkSize.z;
    chunk.chunkX = ChunkX;
    chunk.chunkZ = ChunkZ;
    //chunk.blocks.resize(ChunkSize.x * ChunkSize.y * ChunkSize.z);

    if (game_settings.World_Generation_Type == 3) {
        for (int x = 0; x < ChunkSize.x; x++) {
            for (int z = 0; z < ChunkSize.z; z++) {
                const float worldX = ChunkX * ChunkSize.x + x;
                const float worldZ = ChunkZ * ChunkSize.z + z;

                const float Noise_Biome = (biomeNoise.GetNoise(worldX * BiomeFreq, worldZ * BiomeFreq)+1)*0.5;
                const float biomeFactor = pow(Noise_Biome, BiomePower);

                float baseHeight = 0.0f;
                float frequency = glm::mix(baseFreq, baseFreq*2.0f, Noise_Biome);//baseFreq;
                float amplitude = glm::mix(baseAmp, baseAmp*1.9f, Noise_Biome);//baseAmp;

                for (int i = 0; i < octaves; ++i) {
                    baseHeight += terrainNoise.GetNoise(worldX * frequency, worldZ * frequency) * amplitude;
                    frequency *= AddedFreq;
                    amplitude *= AddedAmp;
                }

                float normalizedHeight = (baseHeight/octaves) * 0.5f + 0.5f;
                normalizedHeight *= (BiomeBase + biomeFactor * BiomeMult);
                const int intHeight = std::max(0, static_cast<int>(normalizedHeight * ChunkSize.y) - 24);

                for (float y = 0; y < ChunkSize.y; y++) {

                    const float val2 = biomeNoise.GetNoise(worldX, y, worldZ) * ChunkSize.y;
                    const int temp = glm::mix(val2/16, val2/4, Noise_Biome) + intHeight;

                    if (y == temp) {
                        chunk.set(x, y, z, Chunk::BlockDefs.at(2)); // Grass
                    } else if (y <= temp - 3) {
                        chunk.set(x, y, z, Chunk::BlockDefs.at(1)); // Stone
                    } else if (y <= temp) {
                        chunk.set(x, y, z, Chunk::BlockDefs.at(3)); // Dirt
                    }
                }
            }
        }
    } else if (game_settings.World_Generation_Type == 2) {
        const float SeaLevel = 64.0f;

        const float Continent_Freq = 0.01;
        const float Continent_Ampl = 80.0f;

        const float Erosion_Freq = 0.1f;
        const float Erosion_Ampl = 1.0f;

        const float Weird_Freq = 0.02f;
        const float Weird_Ampl = 80.0f;

        const float Peaks_Freq = 0.3f;

        for (int x = 0; x < ChunkSize.x; ++x) {
            for (int z = 0; z < ChunkSize.z; ++z) {
                const float wx = ChunkX * ChunkSize.x + x;
                const float wz = ChunkZ * ChunkSize.z + z;

                // Continental
                const float cont_n = Continentalness.GetNoise(wx * Continent_Freq, wz * Continent_Freq);
                const float mountains = cont_n * Continent_Ampl;
                const float mountain_mask = std::clamp((cont_n+0.2f) / 0.8f, 0.0f, 1.0f);

                // Erosion
                const float erosion_n = Erosion.GetNoise(wx * Erosion_Freq, wz * Erosion_Freq);
                const float erosion = (Erosion_Ampl * erosion_n + 1.0f) * 0.5f;

                const float BaseHeight = lerp(SeaLevel, SeaLevel + mountains, 1.0f - erosion_n);
                const float heightFade = std::clamp(1.0f - std::abs(BaseHeight - SeaLevel) / 20.0f, 0.0f, 1.0f);

                float riverMask = 1.0f - std::abs(erosion * 2.0f - 1.0f);
                riverMask = std::clamp((riverMask - 0.95f) / 0.05f, 0.0f, 1.0f);
                riverMask *= mountain_mask;
                riverMask *= heightFade;

                // Weird and Peaks
                const float weird_n = Weidness.GetNoise(wx * Weird_Freq, wz * Weird_Freq);
                const float weird_height = weird_n * Weird_Ampl * mountain_mask;
                const float peaks_n = Peaks.GetNoise(wx * Peaks_Freq, wz * Peaks_Freq);
                const float peakMask = mountain_mask * (1.0f - erosion);

                const float ridged = 1.0f - std::abs(peaks_n);

                float detail = weird_height + ridged * 80.0f * peakMask;
                detail *= lerp(0.3f, 1.0f, 1.0f - erosion);

                float shape = BaseHeight + detail;
                shape -= riverMask * 80.0f;

                const float beachMask = std::clamp(1.0f - std::abs(shape - SeaLevel) / 2.0f, 0.0f, 1.0f);
                for (int y = 0; y < ChunkSize.y; y++) {

                    const float Density = shape - y;
                    if (Density > 0.0f) {
                        if (beachMask > 0)
                            chunk.set(x, y, z, Chunk::BlockDefs.at(3));
                        else
                            chunk.set(x, y, z, Chunk::BlockDefs.at(2)); // Grass
                    } else if (y <= SeaLevel) {
                        chunk.set(x, y, z, Chunk::BlockDefs.at(6));
                    }
                }
            }
        }

    }
    return chunk;
}
