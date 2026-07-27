#include "World/World.hpp"

#include <algorithm>

std::unordered_map<std::pair<int, int>, Chunk, World_Map::PairHash> World_Map::World;
std::vector<World_Map::Render_Info> World_Map::Render_List;
std::vector<Chunk*> World_Map::Mesh_Queue;

void World_Map::Set_Dirty(const int chunkx, const int chunkz) {
    const auto chunk = find_chunk(chunkx, chunkz);
    if (chunk) {
        chunk->DirtyFlag = true;
        chunk->has_mesh = false;
        for (auto& info : Render_List) {
            if (info.chunkX == chunkx && info.chunkZ == chunkz)
                info.Delete = 1;
        }
        if(std::find(Mesh_Queue.begin(), Mesh_Queue.end(), chunk) == Mesh_Queue.end()) {
            Mesh_Queue.push_back(chunk);
        }
    }
}

void World_Map::Set_Neighbors_Dirty(const int localx, const int localz, const int chunkx, const int chunkz) {
    if (localx == 0) Set_Dirty(chunkx-1, chunkz);
    if (localx == Chunk::WIDTH-1) Set_Dirty(chunkx+1, chunkz);
    if (localz == 0) Set_Dirty(chunkx, chunkz-1);
    if (localz == Chunk::DEPTH-1) Set_Dirty(chunkx, chunkz+1);
}
