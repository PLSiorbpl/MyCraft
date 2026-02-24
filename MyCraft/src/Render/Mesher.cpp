#include "MeshManager.hpp"
#include "World/Chunk.hpp"
#include "World/World.hpp"
#include "Utils/Globals.hpp"
#include "Frustum.hpp"
#include <glm/glm.hpp>

using namespace Meshing;

Chunk* getChunk(int x, int z) {
    const std::pair key{x, z};

    const auto it = World_Map::World.find(key);

    if (it != World_Map::World.end())
        return &it->second;

    return nullptr;
}

void Mesher::Manager(const Frustum::Frust &Frust, const glm::ivec3 Chunk_Size) {
    for (Chunk* chunk : World_Map::Mesh_Queue) {
        if (!chunk->DirtyFlag || !chunk->Gen_Mesh || chunk->InRender)
            continue;
        if (game.Updates >= game.Mesh_Updates)
            break;

        const auto chunkMin = glm::vec3(chunk->chunkX * Chunk_Size.x, 0, chunk->chunkZ * Chunk_Size.z);
        const glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk_Size);

        const bool visible = Frustum::IsAABBVisible(Frust, chunkMin, chunkMax);

        // Normal Updates
        if (visible && game.Updates < game.Mesh_Updates) {
            std::lock_guard lock(QueueMutex);

            ChunkSnapshot ss;
            ss.ChunkPos = {chunk->chunkX, chunk->chunkZ};
            ss.blocks = chunk->blocks;

            bool error = false;
            auto copyNeighbor = [&](const int index, const Chunk* n) {
                if (n)
                    ss.neighbor[index] = n->blocks;
                else
                    error = true;
            };

            copyNeighbor(0, getChunk(chunk->chunkX - 1, chunk->chunkZ)); // left
            if (error) continue;
            copyNeighbor(1, getChunk(chunk->chunkX + 1, chunk->chunkZ)); // right
            if (error) continue;
            copyNeighbor(2, getChunk(chunk->chunkX, chunk->chunkZ - 1)); // front
            if (error) continue;
            copyNeighbor(3, getChunk(chunk->chunkX, chunk->chunkZ + 1)); // back
            if (error) continue;

            Queue.push(ss);
            game.Updates++;
            continue;
        }

                // Lazy Updates
        if (!visible && game.Updates < game.Lazy_Mesh_Updates) {
            std::lock_guard lock(QueueMutex);
            Queue.push({{chunk->chunkX, chunk->chunkZ}, chunk->blocks});
            game.Updates++;
        }
    }
}


void Mesher::Meshing_Worker() {
    while (Running) {
        ChunkSnapshot ss;
        {
            std::unique_lock lock(QueueMutex);
            MeshCV.wait(lock, [this]{ return !Running; });

            if (!Running) return;
            // Get everything needed for mesh
            ss = Queue.front();
            Queue.pop();
        }
        // make mesh
        std::vector<Chunk::Vertex> Mesh;

        {
            std::lock_guard lock(FQueueMutex);
            FQueue.push_back(std::move(Mesh));
        }
    }
}