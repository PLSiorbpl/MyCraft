#include "Terrain.hpp"

void TerrainGen::Generate_Terrain_Chunk(int Chunk_x, int Chunk_z, std::map<std::pair<int, int>, Chunk>& World, glm::ivec3 ChunkSize) {
    Chunk Chunk;
    Chunk.width = ChunkSize.x;
    Chunk.height = ChunkSize.y;
    Chunk.depth = ChunkSize.z;
    Chunk.chunkX = Chunk_x;
    Chunk.chunkZ = Chunk_z;
    Chunk.blocks.resize(ChunkSize.x * ChunkSize.y * ChunkSize.z);

    for (int x = 0; x < ChunkSize.x; ++x) {
        for (int z = 0; z < ChunkSize.z; ++z) {
            float worldX = Chunk_x * ChunkSize.x + x;
            float worldZ = Chunk_z * ChunkSize.z + z;

            float biomeFactor = biomeNoise.GetNoise(worldX * 0.003f, worldZ * 0.003f);
            biomeFactor = biomeFactor * 0.5f + 0.5f;

            float baseHeight = 0.0f;
            float frequency = baseFreq;
            float amplitude = baseAmp;

            for (int i = 0; i < octaves; ++i) {
                baseHeight += terrainNoise.GetNoise(worldX * frequency, worldZ * frequency) * amplitude;
                frequency *= 2.0f;
                amplitude *= 0.2f;
            }

            float normalizedHeight = baseHeight * 0.5f + 0.5f;
            normalizedHeight *= (1.5f + biomeFactor * 0.5f);
            int intHeight = static_cast<int>(normalizedHeight * ChunkSize.y);

            for (int y = 0; y <= intHeight && y < ChunkSize.y; ++y) {
                if (abs((Chunk_x + Chunk_z)) % 2 == 1) {
                    Chunk.set(x, y, z, Chunk::BlockDefs.at(1)); // Stone
                } else {
                    Chunk.set(x, y, z, Chunk::BlockDefs.at(2)); // Grass
                }
            }
        }
    }

    World[{Chunk_x, Chunk_z}] = std::move(Chunk);
}

TerrainGen::TerrainGen(int seed) : Seed(seed) {
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    terrainNoise.SetSeed(seed);

    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    biomeNoise.SetSeed(seed + 1337);
}