#include "Breaking.hpp"

void Terrain_Action::RayCastBlock(camera &Camera, const glm::ivec3 ChunkSize, bool Action, int block, float MaxDistance, float StepSize) {
    bool Finish = false;
    auto& World = World_Map::World;
    glm::vec3 Pos;
    glm::vec3 direction;
    direction.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction.y = sin(glm::radians(Camera.Pitch));
    direction.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction = glm::normalize(direction);

    // --------------
    // Functions
    // --------------
    auto SetNeighborsDirty = [&](int localX, int localZ, int chunkX, int chunkZ) {
        auto mark = [&](int cx, int cz){
            auto it = World.find({cx, cz});
            if (it != World.end()) {
                it->second.DirtyFlag = true;
                it->second.Gen_Mesh = true;
            }
        };
        if (localX == 0) mark(chunkX-1, chunkZ);
        if (localX == ChunkSize.x-1) mark(chunkX+1, chunkZ);
        if (localZ == 0) mark(chunkX, chunkZ-1);
        if (localZ == ChunkSize.z-1) mark(chunkX, chunkZ+1);
    };

    for (float Distance = 0; Distance < MaxDistance; Distance += StepSize) {
        Pos = Camera.Position + direction * Distance;

        const glm::ivec3 Block = glm::floor(Pos);

        const int chunkX = floor((float)Block.x / ChunkSize.x);
        const int chunkZ = floor((float)Block.z / ChunkSize.z);

        const int localX = Block.x - chunkX * ChunkSize.x;
        const int localZ = Block.z - chunkZ * ChunkSize.z;

        if (localX < 0 || localX >= ChunkSize.x || localZ < 0 || localZ >= ChunkSize.z) {
            continue;
        }
        // --------------
        // Action
        // --------------
        auto it = World.find({chunkX, chunkZ});
        if (it != World.end()) {
            Chunk& chunk0 = it->second;

            if (Block.y >= 0 && Block.y < ChunkSize.y) {
                if (Action) {
                    // --------------
                    // Breaking
                    // --------------

                    if (chunk0.get(localX, Block.y, localZ).solid) {
                        chunk0.set(localX, Block.y, localZ, Chunk::BlockDefs.at(0));
                        chunk0.DirtyFlag = true;
                        chunk0.Gen_Mesh = true;
                        SetNeighborsDirty(localX, localZ, chunkX, chunkZ);
                        break;
                    }
                } else if (Camera.Place_CoolDown == 0) {
                    // --------------
                    // Placing
                    // --------------

                    const glm::ivec3 PBlock = glm::floor(Camera.Position + direction * (Distance-StepSize));

                    const int PchunkX = floor((float)PBlock.x / ChunkSize.x);
                    const int PchunkZ = floor((float)PBlock.z / ChunkSize.z);

                    const int localPX = PBlock.x - PchunkX * ChunkSize.x;
                    const int localPZ = PBlock.z - PchunkZ * ChunkSize.z;

                    auto itP = World.find({PchunkX, PchunkZ});
                    Chunk& chunk1 = itP->second;

                    if (PBlock.y >= 0 && PBlock.y < ChunkSize.y) {

                        if (chunk0.get(localX, Block.y, localZ).id != 0 && chunk1.get(localPX, PBlock.y, localPZ).id == 0) {
                            const Chunk::Block LastBlock = chunk1.get(localPX, PBlock.y, localPZ);
                            chunk1.set(localPX, PBlock.y, localPZ, Chunk::BlockDefs.at(block));
                            if (Colision.isSolidAround(Camera.Position, ChunkSize)) {
                                // Block is in Player
                                chunk1.set(localPX, PBlock.y, localPZ, LastBlock);
                                break;
                            } else {
                                // Block Placed
                                chunk1.DirtyFlag = true;
                                chunk1.Gen_Mesh = true;
                                Camera.Place_CoolDown = 12;
                                SetNeighborsDirty(localPX, localPZ, PchunkX, PchunkZ);
                                break;
                            }

                        }
                    }
                }
            }
        }
    }
}