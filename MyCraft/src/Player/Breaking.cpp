#include "Breaking.hpp"

void Terrain_Action::RayCastBlock(const camera &Camera, const glm::ivec3 ChunkSize, bool Action, int block, float MaxDistance, float StepSize) {
    bool Finish = false;
    auto& World = World_Map::World;
    glm::vec3 Pos;
    glm::vec3 direction;
    direction.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction.y = sin(glm::radians(Camera.Pitch));
    direction.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction = glm::normalize(direction);

    for (float Distance = 0; Distance < MaxDistance; Distance += StepSize) {
        Pos = Camera.Position + direction * Distance;

        glm::ivec3 Block = glm::floor(Pos);

        const int chunkX = floor((float)Block.x / ChunkSize.x);
        const int chunkZ = floor((float)Block.z / ChunkSize.z);

        const int localX = Block.x - chunkX * ChunkSize.x;
        const int localZ = Block.z - chunkZ * ChunkSize.z;

        if (localX < 0 || localX >= ChunkSize.x || localZ < 0 || localZ >= ChunkSize.z) {
            continue;
        }

        // Logic
        auto it = World.find({chunkX, chunkZ});
        if (it != World.end()) {
            Chunk& chunk = it->second;

            if (Block.y >= 0 && Block.y < ChunkSize.y) {
                // Breaking
                if (Action) {
                    if (chunk.get(localX, Block.y, localZ).solid) {
                        chunk.set(localX, Block.y, localZ, Chunk::BlockDefs.at(0));
                        chunk.DirtyFlag = true;
                        Finish = true;
                    }
                } else {
                    // Placing
                    glm::ivec3 PBlock = glm::floor(Camera.Position + direction * (Distance-StepSize));

                    const int PchunkX = floor((float)PBlock.x / ChunkSize.x);
                    const int PchunkZ = floor((float)PBlock.z / ChunkSize.z);

                    const int localPX = PBlock.x - PchunkX * ChunkSize.x;
                    const int localPZ = PBlock.z - PchunkZ * ChunkSize.z;

                    auto itP = World.find({PchunkX, PchunkZ});
                    Chunk& chunkP = itP->second;

                    if (PBlock.y >= 0 && PBlock.y < ChunkSize.y) {

                        if (chunk.get(localX, Block.y, localZ).id != 0 && chunkP.get(localPX, PBlock.y, localPZ).id == 0) {
                            chunkP.set(localPX, PBlock.y, localPZ, Chunk::BlockDefs.at(block));
                            chunkP.DirtyFlag = true;
                            Finish = true;
                        }
                    }
                }
            }
        }
        // X
        if (localX == 0) {
        auto itN = World.find({chunkX - 1, chunkZ});
        if (itN != World.end()) itN->second.DirtyFlag = true;
        }
        if (localX == ChunkSize.x - 1) {
            auto itN = World.find({chunkX + 1, chunkZ});
            if (itN != World.end()) itN->second.DirtyFlag = true;
        }
        // Z
        if (localZ == 0) {
            auto itN = World.find({chunkX, chunkZ - 1});
            if (itN != World.end()) itN->second.DirtyFlag = true;
        }
        if (localZ == ChunkSize.z - 1) {
            auto itN = World.find({chunkX, chunkZ + 1});
            if (itN != World.end()) itN->second.DirtyFlag = true;
        }
        if (Finish) {
            break;
        }
    }
}