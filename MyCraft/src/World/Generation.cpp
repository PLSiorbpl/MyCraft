#include "Generation.hpp"
#include <glad/glad.h>

void ChunkGeneration::GenerateChunks(const camera &Camera, const glm::ivec3 ChunkSize) {
    auto& World = World_Map::World;
    for (int dx = -Camera.RenderDistance-1; dx <= Camera.RenderDistance+1; ++dx) {
        for (int dz = -Camera.RenderDistance-1; dz <= Camera.RenderDistance+1; ++dz) {
            const int chunkX = Camera.Chunk.x + dx;
            const int chunkZ = Camera.Chunk.z + dz;
            
            const int dist2 = dx*dx + dz*dz;
            if (dist2 > Camera.RenderDistance * Camera.RenderDistance) continue;

            const std::pair<int, int> key = {chunkX, chunkZ};
            
            const int dist = std::max(std::abs(dx), std::abs(dz));
            const bool isEdge = (dist > Camera.RenderDistance);

            if (World.find(key) == World.end()) {
                Terrain.Generate_Terrain_Chunk(chunkX, chunkZ, ChunkSize);
                Chunk* ptr = &World[key];
                World_Map::Mesh_Queue.push_back(ptr);
            }

            auto it = World.find(key);
            if (it != World.end()) {
                it->second.Gen_Mesh = !isEdge;
            }
        }
    }
}

void ChunkGeneration::RemoveChunks(const camera& Camera) {
    auto& World = World_Map::World;
    auto& chunk = World_Map::Mesh_Queue;
    auto& RL = World_Map::Render_List;
    std::vector<std::pair<int,int>> toRemove;

    for (const auto& [key, chunk] : World) {
        const int chunkX = key.first;
        const int chunkZ = key.second;

        const int dx = chunkX - Camera.Chunk.x;
        const int dz = chunkZ - Camera.Chunk.z;

        const int dist = std::max(std::abs(dx), std::abs(dz));

        if (dist > Camera.RenderDistance+1) {
            toRemove.push_back(key);
        }
    }

    for (const auto& key : toRemove) {
        const int cx = key.first;
        const int cz = key.second;
        for (size_t i = 0; i < RL.size(); i++) {
            if (RL[i].chunkX == cx && RL[i].chunkZ == cz) {
                glDeleteBuffers(1, &RL[i].vbo);
                glDeleteVertexArrays(1, &RL[i].vao);
                // Fast delete by moving chunk to back
                RL[i] = RL.back();
                RL.pop_back();
                break;
            }
        }
        for (size_t i = 0; i < chunk.size(); i++) {
            if (chunk[i]->chunkX == cx && chunk[i]->chunkZ == cz) {
                // Fast delete by moving chunk to back
                chunk[i] = chunk.back();
                chunk.pop_back();
                break;
            }
        }
        auto it = World.find(key);
        if (it != World.end()) {
            it->second.RemoveData();
            World.erase(it);
        }
    }
}

ChunkGeneration::ChunkGeneration(int seed, float basefreq, float baseamp, int oct, float addfreq, float addamp, float biomefreq, float biomemult, float biomebase, int biomepower)
    : Terrain(seed, basefreq, baseamp, oct, addfreq, addamp, biomefreq, biomemult, biomebase, biomepower)
{}