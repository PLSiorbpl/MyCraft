#include "Terrain.hpp"
#include "World.hpp"
#include "Utils/Globals.hpp"

Chunk TerrainGen::Generate_Terrain_Chunk(int ChunkX, int ChunkZ, const glm::ivec3 ChunkSize) const {
    //auto& World = World_Map::World;
    Chunk chunk;
    chunk.width = ChunkSize.x;
    chunk.height = ChunkSize.y;
    chunk.depth = ChunkSize.z;
    chunk.chunkX = ChunkX;
    chunk.chunkZ = ChunkZ;
    chunk.blocks.resize(ChunkSize.x * ChunkSize.y * ChunkSize.z);

    if (game_settings.Generation_Type == 3) {
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
    } else if (game_settings.Generation_Type == 2) {
        for (int x = 0; x < ChunkSize.x; ++x) {
            for (int z = 0; z < ChunkSize.z; ++z) {
                const float worldX = ChunkX * ChunkSize.x + x;
                const float worldZ = ChunkZ * ChunkSize.z + z;

                const float Noise_Biome = biomeNoise.GetNoise(worldX * BiomeFreq, worldZ * BiomeFreq) * BiomePower;

                float baseHeight = 0.0f;
                float frequency = glm::mix(baseFreq, baseFreq*BiomeBase, Noise_Biome);
                float amplitude = glm::mix(baseAmp, baseAmp*BiomeMult, Noise_Biome);

                for (int i = 0; i < octaves; ++i) {
                    baseHeight += terrainNoise.GetNoise(worldX * frequency, worldZ * frequency) * amplitude;
                    frequency *= AddedFreq;
                    amplitude *= AddedAmp;
                }

                const float normalizedHeight = (baseHeight/octaves) * 0.5f + 0.5f;

                const int intHeight = std::max(0, static_cast<int>(normalizedHeight * ChunkSize.y) - 32);

                for (int y = 0; y <= intHeight && y < ChunkSize.y; ++y) {
                    if (y == intHeight) {
                        chunk.set(x, y, z, Chunk::BlockDefs.at(2)); // Grass
                    } else if (y >= intHeight - 3) {
                        chunk.set(x, y, z, Chunk::BlockDefs.at(3)); // Dirt
                    } else {
                        chunk.set(x, y, z, Chunk::BlockDefs.at(1)); // Stone
                    }
                }
            }
        }
    }
    return chunk;
   //World[{ChunkX, ChunkZ}] = std::move(Chunk);
}
