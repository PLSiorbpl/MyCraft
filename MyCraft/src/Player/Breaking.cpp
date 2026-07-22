#include "Breaking.hpp"
#include <algorithm>

#include "World/World.hpp"

void Terrain_Action::RayCastBlock(camera &Camera, int Action, int block, Selection& Sel, float MaxDistance, float StepSize) {
    auto &World = World_Map::World;

    auto SetNeighborsDirty = [&](int localX, int localZ, int chunkX, int chunkZ) {
        auto mark = [&](int cx, int cz) {
            auto it = World.find({cx, cz});
            if (it != World.end()) {
                it->second.DirtyFlag = true;
                it->second.has_mesh = false;
                for (auto& info : World_Map::Render_List) {
                    if (info.chunkX == cx && info.chunkZ == cz) {
                        info.Delete = 1;
                    }
                }
                Chunk* chunkPtr = &it->second;
                if(std::find(World_Map::Mesh_Queue.begin(), World_Map::Mesh_Queue.end(), chunkPtr) == World_Map::Mesh_Queue.end()) {
                    World_Map::Mesh_Queue.push_back(chunkPtr);
                }
            }
        };
        if (localX == 0)             mark(chunkX-1, chunkZ);
        if (localX == Chunk::WIDTH-1) mark(chunkX+1, chunkZ);
        if (localZ == 0)             mark(chunkX, chunkZ-1);
        if (localZ == Chunk::DEPTH-1) mark(chunkX, chunkZ+1);
    };

    glm::vec3 direction;
    direction.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction.y = sin(glm::radians(Camera.Pitch));
    direction.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction = glm::normalize(direction);

    const glm::vec3 Pos = Camera.Position;
    glm::ivec3 c_block = glm::floor(Pos);

    const glm::ivec3 Step = glm::ivec3(direction.x > 0 ? 1 : -1,
        direction.y > 0 ? 1 : -1,
        direction.z > 0 ? 1 : -1);

    glm::vec3 safeDir = direction;
    constexpr glm::vec3 eps(1e-6f);
        
    if (fabs(safeDir.x) < eps.x) safeDir.x = (safeDir.x >= 0 ? eps.x : -eps.x);
    if (fabs(safeDir.y) < eps.x) safeDir.y = (safeDir.y >= 0 ? eps.x : -eps.x);
    if (fabs(safeDir.z) < eps.x) safeDir.z = (safeDir.z >= 0 ? eps.x : -eps.x);
    
    glm::vec3 tMax = glm::vec3(((Step.x > 0 ? (c_block.x + 1) : c_block.x) - Pos.x) / safeDir.x,
        ((Step.y > 0 ? (c_block.y + 1) : c_block.y) - Pos.y) / safeDir.y,
        ((Step.z > 0 ? (c_block.z + 1) : c_block.z) - Pos.z) / safeDir.z);

    const glm::vec3 tDelta = glm::abs(1.0f / (direction + eps));

    
    float distance = 0.0f;
    bool firstrun = true;
    Chunk* LastChunk = nullptr;
    Block *LastBlock;
    glm::ivec3 LastCord;
    glm::ivec2 LastC;

    // RayCast
    while(distance < MaxDistance) {
        if (distance > MaxDistance) break;
        const int cx = floor(c_block.x / static_cast<float>(Chunk::WIDTH));
        const int cz = floor(c_block.z / static_cast<float>(Chunk::DEPTH));

        const int LocalX = c_block.x - cx * Chunk::WIDTH;
        const int LocalZ = c_block.z - cz * Chunk::DEPTH;
        
        auto it = World.find({cx, cz});
        if (it != World.end()) {
            Chunk& chunk = it->second;

            // Actions  Break | Place | Show SelectionBox
            if (c_block.y >= 0 && c_block.y < Chunk::HEIGHT) {
                // Break
                if (Action == 1 && Camera.Break_CoolDown == 0) {
                    if (chunk.get_state(LocalX, c_block.y, LocalZ)->is_solid) {
                        chunk.set(LocalX, c_block.y, LocalZ, Chunk::block(block_type::Air));
                        chunk.DirtyFlag = true;
                        chunk.has_mesh = false;
                        for (auto& info : World_Map::Render_List) {
                            if (info.chunkX == cx && info.chunkZ == cz) {
                                info.Delete = 1;
                            }
                        }
                        Chunk* chunkPtr = &it->second;
                        if(std::find(World_Map::Mesh_Queue.begin(), World_Map::Mesh_Queue.end(), chunkPtr) == World_Map::Mesh_Queue.end()) {
                            World_Map::Mesh_Queue.push_back(chunkPtr);
                        }
                        SetNeighborsDirty(LocalX, LocalZ, cx, cz);
                        Camera.Break_CoolDown = 8;
                        break;
                    }
                    // Place
                } else if (Action == 2 && Camera.Place_CoolDown == 0 && !firstrun) {
                    if (chunk.get_state(LocalX, c_block.y, LocalZ)->is_solid && !LastBlock->is_solid) {
                        const Chunk::block TryBlock = LastChunk->get(LastCord.x, LastCord.y, LastCord.z);
                        LastChunk->set(LastCord.x, LastCord.y, LastCord.z, Chunk::block(static_cast<block_type>(block)));
                        if (Colision.isSolidAround(Camera.Position)) {
                            LastChunk->set(LastCord.x, LastCord.y, LastCord.z, TryBlock);
                            Camera.Place_CoolDown = 8;
                            break;
                        } else {
                            LastChunk->DirtyFlag = true;
                            LastChunk->has_mesh = false;
                            for (auto& info : World_Map::Render_List) {
                                if (info.chunkX == LastC.x && info.chunkZ == LastC.y) { // vec2 so x and y but y is z
                                    info.Delete = 1;
                                }
                            }
                            Chunk* chunkPtr = LastChunk;
                            if(std::find(World_Map::Mesh_Queue.begin(), World_Map::Mesh_Queue.end(), chunkPtr) == World_Map::Mesh_Queue.end()) {
                                World_Map::Mesh_Queue.push_back(chunkPtr);
                            }
                            //SetNeighborsDirty(LocalX, LocalZ, cx, cz);
                            SetNeighborsDirty(LastCord.x, LastCord.z, LastC.x, LastC.y);
                            Camera.Place_CoolDown = 12;
                            break;
                        }
                    }
                    // Show SelectionBox
                } else if (Action == 0) {
                    if (chunk.get_state(LocalX, c_block.y, LocalZ)->is_solid) {
                        Sel.Draw(glm::vec3(c_block));
                        Camera.Draw_Selection = true;
                        break;
                    } else {
                        Camera.Draw_Selection = false;
                    }
                }
            LastChunk = &chunk;
            LastBlock = chunk.get_state(LocalX, c_block.y, LocalZ);
            }
            LastCord = glm::ivec3(LocalX, c_block.y, LocalZ);
            LastC = glm::ivec2(cx, cz);
            firstrun = false;
        }
        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                c_block.x += Step.x;
                tMax.x += tDelta.x;
            } else {
                c_block.z += Step.z;
                tMax.z += tDelta.z;
            }
        } else {
            if (tMax.y < tMax.z) {
                c_block.y += Step.y;
                tMax.y += tDelta.y;
            } else {
                c_block.z += Step.z;
                tMax.z += tDelta.z;
            }
        }

        distance = std::min({tMax.x, tMax.y, tMax.z});
    }
}