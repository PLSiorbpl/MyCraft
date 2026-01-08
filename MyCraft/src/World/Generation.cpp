#include <glad/glad.h>
#include "Generation.hpp"
#include <iostream>

void ChunkGeneration::LookForChunks() {
    auto& World = World_Map::World;
    const int Rd = Camera.RenderDistance;
    for (int dx = -Rd-1; dx <= Rd+1; ++dx) {
        for (int dz = -Rd-1; dz <= Rd+1; ++dz) {

            const int chunkX = Camera.Chunk.x + dx;
            const int chunkZ = Camera.Chunk.z + dz;
            const std::pair<int, int> key = {chunkX, chunkZ};
            const int dist = std::max(std::abs(dx), std::abs(dz));
            const bool isEdge = (dist > Rd);

            {
                std::lock_guard lock(GenMutex);
                if (World.find(key) == World.end() && GeneratingChunks.find(key) == GeneratingChunks.end()) {
                    GenQueue.push(key);
                    GeneratingChunks.insert(key);
                }
            }

            auto it = World.find(key);
            if (it != World.end()) {
                it->second.Gen_Mesh = !isEdge;
            }
        }
    }
    GenCV.notify_one();
}

void ChunkGeneration::GenerateChunk(const glm::ivec3 ChunkSize) {
    while (Running) {
        std::pair<int,int> chunkPos;

        {
            std::unique_lock lock(GenMutex);
            GenCV.wait(lock, [this]{ return !GenQueue.empty() || !Running; });

            if (!Running) return;
            chunkPos = GenQueue.front();
            GenQueue.pop();
        }

        Chunk data = Terrain.Generate_Terrain_Chunk(chunkPos.first, chunkPos.second, ChunkSize);

        {
            std::lock_guard lock(ResultMutex);
            ReadyChunks.push_back(std::move(data));
        }
        {
            std::lock_guard lock(GenMutex);
            GeneratingChunks.erase(chunkPos);
        }
    }
}


void ChunkGeneration::RemoveChunks() {
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
    if (!RL.empty()) glFinish();
    for (const auto& key : toRemove) {
        const int cx = key.first;
        const int cz = key.second;
        if (!RL.empty()) {
            for (int i = RL.size(); i-- > 0;) {
                if (RL[i].chunkX == cx && RL[i].chunkZ == cz) {
                    auto& chunk1 = World_Map::World.find({RL[i].chunkX, RL[i].chunkZ})->second;
                    chunk1.InRender = false;
                    glDeleteBuffers(1, &RL[i].vbo);
                    glDeleteVertexArrays(1, &RL[i].vao);
                    // Fast delete by moving chunk to back
                    RL[i] = RL.back();
                    RL.pop_back();
                    break;
                }
            }
        }

        if (!chunk.empty()){
            for (int i = chunk.size(); i-- > 0;) {
                if (chunk[i]->chunkX == cx && chunk[i]->chunkZ == cz) {
                    // Fast delete by moving chunk to back
                    chunk[i] = chunk.back();
                    chunk.pop_back();
                    break;
                }
            }
        }

        auto it = World.find(key);
        if (it != World.end()) {
            it->second.RemoveData();
            World.erase(it);
        }
    }
}

ChunkGeneration::ChunkGeneration(int seed, float basefreq, float baseamp, int oct, float addfreq, float addamp, float biomefreq, float biomemult, float biomebase, float biomepower)
    : Terrain(seed, basefreq, baseamp, oct, addfreq, addamp, biomefreq, biomemult, biomebase, biomepower)
{}