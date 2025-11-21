#include "Breaking.hpp"

void Terrain_Action::RayCastBlock(camera &Camera, const glm::ivec3 ChunkSize, int Action, int block, Selection& Sel, bool &ChunkUpdated, float MaxDistance, float StepSize) {
    auto &World = World_Map::World;

    auto SetNeighborsDirty = [&](int localX, int localZ, int chunkX, int chunkZ) {
        auto mark = [&](int cx, int cz) {
            auto it = World.find({cx, cz});
            if (it != World.end()) {
                it->second.DirtyFlag = true;
                it->second.Gen_Mesh = true;
                it->second.Ready_Render = false;
                // we need to delete chunk from Render List
                ChunkUpdated = true;
                const std::pair<int, int> key = {cx, cz};
                Chunk* ptr = &it->second;
                World_Map::Mesh_Queue.push_back(ptr);
            }
        };
        if (localX == 0) mark(chunkX-1, chunkZ);
        if (localX == ChunkSize.x-1) mark(chunkX+1, chunkZ);
        if (localZ == 0) mark(chunkX, chunkZ-1);
        if (localZ == ChunkSize.z-1) mark(chunkX, chunkZ+1);
    };

    glm::vec3 direction;
    direction.x = cos(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction.y = sin(glm::radians(Camera.Pitch));
    direction.z = sin(glm::radians(Camera.Yaw)) * cos(glm::radians(Camera.Pitch));
    direction = glm::normalize(direction);

    const glm::vec3 Pos = Camera.Position;
    glm::ivec3 Block = glm::floor(Pos);

    const glm::ivec3 Step = glm::ivec3(direction.x > 0 ? 1 : -1,
        direction.y > 0 ? 1 : -1,
        direction.z > 0 ? 1 : -1);
    
    glm::vec3 tMax = glm::vec3(((Step.x > 0 ? (Block.x + 1) : Block.x) - Pos.x) / direction.x,
        ((Step.y > 0 ? (Block.y + 1) : Block.y) - Pos.y) / direction.y,
        ((Step.z > 0 ? (Block.z + 1) : Block.z) - Pos.z) / direction.z);

    const glm::vec3 tDelta = glm::abs(1.0f / direction+0.000001f);
    
    float distance = 0.0f;
    bool firstrun = true;
    Chunk* LastChunk = nullptr;
    Chunk::Block LastBlock;
    glm::ivec3 LastCord;
    glm::ivec2 LastC;

    while(distance < MaxDistance) {
        if (distance > MaxDistance) break;
        const int cx = floor(Block.x / float(ChunkSize.x));
        const int cz = floor(Block.z / float(ChunkSize.z));

        const int LocalX = Block.x - cx * ChunkSize.x;
        const int LocalZ = Block.z - cz * ChunkSize.z;
        
        auto it = World.find({cx, cz});
        if (it != World.end()) {
            Chunk& chunk = it->second;

            if (Block.y >= 0 && Block.y < ChunkSize.y) {
                if (Action == 1 && Camera.Break_CoolDown == 0) {
                    if (chunk.get(LocalX, Block.y, LocalZ).Flags & 0b10'00'00'00) {
                        chunk.set(LocalX, Block.y, LocalZ, Chunk::BlockDefs.at(0));
                        chunk.DirtyFlag = true;
                        chunk.Gen_Mesh = true;
                        chunk.Ready_Render = false;
                        // we need to delete chunk from Render List
                        ChunkUpdated = true;
                        Chunk* ptr = &it->second;
                        World_Map::Mesh_Queue.push_back(ptr);
                        SetNeighborsDirty(LocalX, LocalZ, cx, cz);
                        Camera.Break_CoolDown = 8;
                        break;
                    }
                } else if (Action == 2 && Camera.Place_CoolDown == 0 && !firstrun) {
                    if (chunk.get(LocalX, Block.y, LocalZ).Flags & 0b10'00'00'00 && !(LastBlock.Flags & 0b10'00'00'00)) {
                        const Chunk::Block TryBlock = LastChunk->get(LastCord.x, LastCord.y, LastCord.z);
                        LastChunk->set(LastCord.x, LastCord.y, LastCord.z, Chunk::BlockDefs.at(block));
                        if (Colision.isSolidAround(Camera.Position, ChunkSize)) {
                            LastChunk->set(LastCord.x, LastCord.y, LastCord.z, TryBlock);
                            Camera.Place_CoolDown = 8;
                            break;
                        } else {
                            LastChunk->DirtyFlag = true;
                            LastChunk->Gen_Mesh = true;
                            LastChunk->Ready_Render = false;
                            // we need to delete chunk from Render List
                            ChunkUpdated = true;
                            Chunk* ptr = &it->second;
                            World_Map::Mesh_Queue.push_back(ptr);
                            SetNeighborsDirty(LastCord.x, LastCord.z, LastC.x, LastC.y);
                            Camera.Place_CoolDown = 12;
                            break;
                        }
                    }
                } else if (Action == 0) {
                    if (chunk.get(LocalX, Block.y, LocalZ).Flags & 0b10'00'00'00) {
                        Sel.Draw(glm::vec3(Block));
                        Camera.Draw_Selection = true;
                        break;
                    } else {
                        Camera.Draw_Selection = false;
                    }
                }
            }
            LastChunk = &chunk;
            LastBlock = chunk.get(LocalX, Block.y, LocalZ);
            LastCord = glm::ivec3(LocalX, Block.y, LocalZ);
            LastC = glm::ivec2(cx, cz);
            firstrun = false;
        }
        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                Block.x += Step.x;
                tMax.x += tDelta.x;
            } else {
                Block.z += Step.z;
                tMax.z += tDelta.z;
            }
        } else {
            if (tMax.y < tMax.z) {
                Block.y += Step.y;
                tMax.y += tDelta.y;
            } else {
                Block.z += Step.z;
                tMax.z += tDelta.z;
            }
        }

        distance = std::min({tMax.x, tMax.y, tMax.z});
    }
}