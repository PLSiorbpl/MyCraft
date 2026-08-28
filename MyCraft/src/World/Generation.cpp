#include <glad/glad.h>
#include "Generation.hpp"
#include <iostream>
#include <ranges>

#include "Mesh/Mesh.hpp"
#include "Render/Camera.hpp"

ChunkGeneration GenerateChunk;

void ChunkGeneration::LookForChunks() {
    const int Rd = Camera.RenderDistance;
    for (int dx = -Rd-1; dx <= Rd+1; ++dx) {
        for (int dz = -Rd-1; dz <= Rd+1; ++dz) {

            const int chunkX = Camera.Chunk.x + dx;
            const int chunkZ = Camera.Chunk.z + dz;
            const std::pair key = {chunkX, chunkZ};
            const int dist = std::max(std::abs(dx), std::abs(dz));
            const bool isEdge = dist > Rd;

            {
                if (!World_Map::World.contains(key) && !GeneratingChunks.contains(key)) {
                    std::lock_guard lock(GenMutex);
                    GenQueue.push(key);
                    GeneratingChunks.insert(key);
                }
            }

            auto *chunk = World_Map::find_chunk(key.first, key.second);
            if (chunk) {
                chunk->is_edge = isEdge;
                if (chunk->is_edge) continue;

                if (!chunk->pending_mesh && !chunk->has_mesh && !chunk->InRender) {
                    chunk->DirtyFlag = true;
                    chunk->pending_mesh = true;
                    mesher.pendingChunks.push_back(chunk);
                }
            }
        }
    }
    GenCV.notify_all();
}

void ChunkGeneration::GenerateChunk() {
    while (Running) {
        std::pair<int,int> chunkPos;

        {
            std::unique_lock lock(GenMutex);
            GenCV.wait(lock, [this]{ return !GenQueue.empty() || !Running; });

            if (!Running) return;
            chunkPos = GenQueue.front();
            GenQueue.pop();
        }

        auto data = std::make_unique<Chunk>();

        Terrain.Generate_Terrain_Chunk(*data, chunkPos.first, chunkPos.second);

        {
            std::lock_guard lock(ResultMutex);
            ReadyChunks.push_back(std::move(data));
        }
    }
}


void ChunkGeneration::RemoveChunks() {
    std::vector<decltype(World_Map::World)::iterator> toRemove;

    for (auto it = World_Map::World.begin(); it != World_Map::World.end(); ++it) {
        const auto& key = it->first;
        const int chunkX = key.first;
        const int chunkZ = key.second;

        const int dx = chunkX - Camera.Chunk.x;
        const int dz = chunkZ - Camera.Chunk.z;

        const int dist = std::max(std::abs(dx), std::abs(dz));

        if (dist > Camera.RenderDistance+1 && !it->second->in_mesher) {
            toRemove.push_back(it);
        }
    }

    for (auto it : toRemove) {
        it->second->RemoveData();
        World_Map::World.erase(it);
    }

    for (int i = World_Map::Render_List.size(); i-- > 0;) {
        auto &info = World_Map::Render_List[i];
        const int chunkX = info.chunkX;
        const int chunkZ = info.chunkZ;

        const int dx = chunkX - Camera.Chunk.x;
        const int dz = chunkZ - Camera.Chunk.z;

        const int dist = std::max(std::abs(dx), std::abs(dz));

        if (dist > Camera.RenderDistance+1) {
            auto chunk = World_Map::find_chunk(chunkX, chunkZ);
            if (chunk) chunk->InRender = false;
            glDeleteBuffers(1, &info.vbo);
            glDeleteVertexArrays(1, &info.vao);
            info = World_Map::Render_List.back();
            World_Map::Render_List.pop_back();
        }
    }

    for (int i = mesher.pendingChunks.size(); i-- > 0;) {
        auto chunk = mesher.pendingChunks[i];

        const int chunkX = chunk->chunkX;
        const int chunkZ = chunk->chunkZ;

        const int dx = chunkX - Camera.Chunk.x;
        const int dz = chunkZ - Camera.Chunk.z;

        const int dist = std::max(std::abs(dx), std::abs(dz));

        if (dist > Camera.RenderDistance+1) {
            mesher.pendingChunks[i] = mesher.pendingChunks.back();
            mesher.pendingChunks.pop_back();
        }
    }
}