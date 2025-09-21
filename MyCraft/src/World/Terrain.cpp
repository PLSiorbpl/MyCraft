#include "Terrain.hpp"

void TerrainGen::Generate_Terrain_Chunk(int Chunk_x, int Chunk_z, glm::ivec3 ChunkSize) {
    auto& World = World_Map::World;
    Chunk Chunk;
    Chunk.width = ChunkSize.x;
    Chunk.height = ChunkSize.y;
    Chunk.depth = ChunkSize.z;
    Chunk.chunkX = Chunk_x;
    Chunk.chunkZ = Chunk_z;
    Chunk.blocks.resize(ChunkSize.x * ChunkSize.y * ChunkSize.z);

    for (int x = 0; x < ChunkSize.x; ++x) {
        for (int z = 0; z < ChunkSize.z; ++z) {
            const float worldX = Chunk_x * ChunkSize.x + x;
            const float worldZ = Chunk_z * ChunkSize.z + z;

            float Noise_Biome = (biomeNoise.GetNoise(worldX * BiomeFreq, worldZ * BiomeFreq)+1)*0.5;
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

            for (int y = 0; y <= intHeight && y < ChunkSize.y; ++y) {
                if (y == intHeight) {
                    Chunk.set(x, y, z, Chunk::BlockDefs.at(2)); // Grass
                } else if (y >= intHeight - 3) {
                    Chunk.set(x, y, z, Chunk::BlockDefs.at(3)); // Dirt
                } else {
                    Chunk.set(x, y, z, Chunk::BlockDefs.at(1)); // Stone
                }
            }
        }
    }

    World[{Chunk_x, Chunk_z}] = std::move(Chunk);
}