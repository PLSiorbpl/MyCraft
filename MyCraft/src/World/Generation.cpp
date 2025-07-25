#include "Generation.hpp"

void ChunkGeneration::GenerateChunks(camera &Camera, std::map<std::pair<int, int>, Chunk>& World, glm::ivec3 ChunkSize) {
    for (int dx = -Camera.RenderDistance; dx <= Camera.RenderDistance; ++dx) {
        for (int dz = -Camera.RenderDistance; dz <= Camera.RenderDistance; ++dz) {
            int chunkX = Camera.Chunk.x + dx;
            int chunkZ = Camera.Chunk.z + dz;

            std::pair<int, int> key = {chunkX, chunkZ};

            if (World.find(key) == World.end()) {
                Terrain.Generate_Terrain_Chunk(chunkX, chunkZ, World, ChunkSize);
            }
        }
    }
}

void ChunkGeneration::RemoveChunks(camera& Camera, std::map<std::pair<int, int>, Chunk>& World) {
    std::vector<std::pair<int,int>> toRemove;

    for (const auto& [key, chunk] : World) {
        int chunkX = key.first;
        int chunkZ = key.second;

        int dx = chunkX - Camera.Chunk.x;
        int dz = chunkZ - Camera.Chunk.z;

        int dist = std::max(std::abs(dx), std::abs(dz));

        if (dist > Camera.RenderDistance) {
            toRemove.push_back(key);
        }
    }

    for (const auto& key : toRemove) {
        World.erase(key);
    }
}

ChunkGeneration::ChunkGeneration(int seed)
    : Terrain(seed) // <--- inicjalizacja TerrainGen
{}