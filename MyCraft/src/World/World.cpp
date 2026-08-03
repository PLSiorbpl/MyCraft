#include "World/World.hpp"

#include <algorithm>

namespace World_Map {
    std::unordered_map<std::pair<int, int>, Chunk, PairHash> World;
    std::vector<Render_Info> Render_List;
    std::vector<Chunk*> Mesh_Queue;

    void Set_Dirty(const int chunkx, const int chunkz) {
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

    void Set_Neighbors_Dirty(const int localx, const int localz, const int chunkx, const int chunkz) {
        if (localx == 0) Set_Dirty(chunkx-1, chunkz);
        if (localx == Chunk::WIDTH-1) Set_Dirty(chunkx+1, chunkz);
        if (localz == 0) Set_Dirty(chunkx, chunkz-1);
        if (localz == Chunk::DEPTH-1) Set_Dirty(chunkx, chunkz+1);
    }
}
