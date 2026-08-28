#include "Mesh.hpp"

#include "World/World.hpp"
#include "World/Mesh.hpp"

Mesher mesher;

void Mesher::mesh_worker() {
    while (running) {
        Chunk *chunk = nullptr;

        {
            std::unique_lock lock(meshInMutex);
            meshCV.wait(lock, [this]{ return !meshQueue.empty() || !running; });

            if (!running) return;
            if (meshQueue.empty()) continue;
            auto [fst, snd] = meshQueue.front();
            meshQueue.pop();
            chunk = World_Map::find_chunk(fst, snd);
        }

        if (!chunk) { fallback(chunk, result::Invalid_ptr); continue; }
        if (!chunk->has_terrain || chunk->is_edge || !chunk->DirtyFlag) { fallback(chunk, result::Bad_Flags); continue; }
        if (World_Map::find_chunk(chunk->chunkX, chunk->chunkZ + 1) == nullptr) { fallback(chunk, result::Missing_N); continue; }
        if (World_Map::find_chunk(chunk->chunkX, chunk->chunkZ - 1) == nullptr) { fallback(chunk, result::Missing_N); continue; }
        if (World_Map::find_chunk(chunk->chunkX + 1, chunk->chunkZ) == nullptr) { fallback(chunk, result::Missing_N); continue; }
        if (World_Map::find_chunk(chunk->chunkX - 1, chunk->chunkZ) == nullptr) { fallback(chunk, result::Missing_N); continue; }

        if (!World_Map::find_chunk(chunk->chunkX, chunk->chunkZ + 1)->has_terrain) { fallback(chunk, result::Missing_N); continue; }
        if (!World_Map::find_chunk(chunk->chunkX, chunk->chunkZ - 1)->has_terrain) { fallback(chunk, result::Missing_N); continue; }
        if (!World_Map::find_chunk(chunk->chunkX + 1, chunk->chunkZ)->has_terrain) { fallback(chunk, result::Missing_N); continue; }
        if (!World_Map::find_chunk(chunk->chunkX - 1, chunk->chunkZ)->has_terrain) { fallback(chunk, result::Missing_N); continue; }

        mesh_t mesh = {};
        mesh.chunkX = chunk->chunkX;
        mesh.chunkZ = chunk->chunkZ;
        mesh.R = result::Done;

        Mesh::GenerateMesh(*chunk, &mesh.mesh);

        {
            std::lock_guard lock(meshOutMutex);
            meshOutQueue.push_back(std::move(mesh));
        }
    }
}

void Mesher::fallback(const Chunk *chunk, const result r) {
    mesh_t mesh = {};
    if (r != result::Invalid_ptr) {
        mesh.chunkX = chunk->chunkX;
        mesh.chunkZ = chunk->chunkZ;
    }
    mesh.R = r;
    {
        std::lock_guard lock(meshOutMutex);
        meshOutQueue.push_back(std::move(mesh));
    }
}
