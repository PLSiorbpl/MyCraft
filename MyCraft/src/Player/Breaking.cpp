#include "Breaking.hpp"
#include <algorithm>

#include "Colisions.hpp"
#include "World/World.hpp"
#include "Tick/Tick.hpp"

void Terrain_Action::RayCastBlock(camera &Camera, int Action, int block, Selection& Sel, float MaxDistance, float StepSize) {
    auto &World = World_Map::World;

    glm::vec3 direction;
    direction.x = std::cos(glm::radians(Camera.Yaw)) * std::cos(glm::radians(Camera.Pitch));
    direction.y = std::sin(glm::radians(Camera.Pitch));
    direction.z = std::sin(glm::radians(Camera.Yaw)) * std::cos(glm::radians(Camera.Pitch));
    direction = glm::normalize(direction);

    const glm::vec3 Pos = Camera.Position;
    glm::ivec3 c_block = glm::floor(Pos);

    const auto Step = glm::ivec3(direction.x > 0 ? 1 : -1,
        direction.y > 0 ? 1 : -1,
        direction.z > 0 ? 1 : -1);

    glm::vec3 safeDir = direction;
    constexpr glm::vec3 eps(1e-6f);
        
    if (fabs(safeDir.x) < eps.x) safeDir.x = (safeDir.x >= 0 ? eps.x : -eps.x);
    if (fabs(safeDir.y) < eps.x) safeDir.y = (safeDir.y >= 0 ? eps.x : -eps.x);
    if (fabs(safeDir.z) < eps.x) safeDir.z = (safeDir.z >= 0 ? eps.x : -eps.x);
    
    glm::vec3 tMax = glm::vec3(((Step.x > 0 ? static_cast<float>(c_block.x) + 1 : static_cast<float>(c_block.x)) - Pos.x) / safeDir.x,
        ((Step.y > 0 ? static_cast<float>(c_block.y) + 1 : static_cast<float>(c_block.y)) - Pos.y) / safeDir.y,
        ((Step.z > 0 ? static_cast<float>(c_block.z) + 1 : static_cast<float>(c_block.z)) - Pos.z) / safeDir.z);

    const glm::vec3 tDelta = glm::abs(1.0f / (direction + eps));

    
    float distance = 0.0f;
    bool firstrun = true;
    Chunk* LastChunk = nullptr;
    Block *LastBlock = nullptr;
    glm::ivec3 LastCord{};
    glm::ivec2 LastC{};

    // RayCast
    while(distance < MaxDistance) {
        if (distance > MaxDistance) break;
        const int cx = std::floor(c_block.x / static_cast<float>(Chunk::WIDTH));
        const int cz = std::floor(c_block.z / static_cast<float>(Chunk::DEPTH));

        const int LocalX = c_block.x - cx * Chunk::WIDTH;
        const int LocalZ = c_block.z - cz * Chunk::DEPTH;
        
        auto it = World.find({cx, cz});
        if (it != World.end()) {
            Chunk& chunk = *it->second;

            // Actions:  Break | Place | Interact | Show SelectionBox
            if (c_block.y >= 0 && c_block.y < Chunk::HEIGHT - 1) {
                if (Action == 1 && Camera.Break_CoolDown == 0) {
                    // -------------------------------------
                    // Breaking block
                    if (chunk.get_state(LocalX, c_block.y, LocalZ)->is_solid) {
                        chunk.get_state(LocalX, c_block.y, LocalZ)->onRemove({LocalX, c_block.y, LocalZ}, {cx, cz});
                        chunk.set(LocalX, c_block.y, LocalZ, Chunk::block(block_type::Air));

                        World_Map::notifyNeighborBlocksConduct({LocalX, c_block.y, LocalZ}, {cx, cz});

                        World_Map::Set_Dirty(cx, cz);
                        World_Map::Set_Neighbors_Dirty(LocalX, LocalZ, cx, cz);

                        Camera.Break_CoolDown = 8;
                        RayCastBlock(Camera, 0, 0, Sel, MaxDistance, StepSize);
                        break;
                    }
                } else if (Action == 2 && Camera.Place_CoolDown == 0 && !firstrun) {
                    // -------------------------------------
                    // Placing/Interacting block
                    if (chunk.get_state(LocalX, c_block.y, LocalZ)->is_solid && !LastBlock->is_solid) {
                        if (block == 0) {
                            chunk.create_state(LocalX, c_block.y, LocalZ);
                            chunk.get_state(LocalX, c_block.y, LocalZ)->onInteraction({LocalX, c_block.y, LocalZ}, {cx, cz});

                            World_Map::notifyNeighborBlocksConduct({LocalX, c_block.y, LocalZ}, {cx, cz});

                            World_Map::Set_Dirty(cx, cz);
                            World_Map::Set_Neighbors_Dirty(LocalX, LocalZ, cx, cz);

                            Camera.Place_CoolDown = 8;
                            RayCastBlock(Camera, 0, 0, Sel, MaxDistance, StepSize);
                            break;
                        }

                        const Chunk::block TryBlock = LastChunk->get(LastCord.x, LastCord.y, LastCord.z);
                        LastChunk->set(LastCord.x, LastCord.y, LastCord.z, Chunk::block(static_cast<block_type>(block)));

                        if (colisions::isSolidAround(Camera.Position)) {
                            LastChunk->set(LastCord.x, LastCord.y, LastCord.z, TryBlock);
                            Camera.Place_CoolDown = 8;
                            break;
                        } else {
                            LastChunk->create_state(LastCord.x, LastCord.y, LastCord.z);
                            LastChunk->get_state(LastCord.x, LastCord.y, LastCord.z)->onPlace(LastCord, LastC, GetDirection(direction));

                            Tick::Instant_queue.push({LastCord, LastC});
                            World_Map::notifyNeighborBlocksConduct(LastCord, LastC);

                            World_Map::Set_Dirty(LastC.x, LastC.y);
                            World_Map::Set_Neighbors_Dirty(LastCord.x, LastCord.z, LastC.x, LastC.y);

                            Camera.Place_CoolDown = 12;
                            RayCastBlock(Camera, 0, 0, Sel, MaxDistance, StepSize);
                            break;
                        }
                    }
                    // Show SelectionBox
                } else if (Action == 0) {
                    Camera.looking_at = chunk.get_state(LocalX, c_block.y, LocalZ);
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
            LastCord = glm::ivec3(LocalX, c_block.y, LocalZ);
            LastC = glm::ivec2(cx, cz);
            firstrun = false;
            }
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