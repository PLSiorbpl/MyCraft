#pragma once
#include <glm/glm.hpp>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_set>
#include <iostream>

#include "World.hpp"
#include "Terrain.hpp"
# include "Utils/Globals.hpp"

class ChunkGeneration {
private:
    int chunkX, chunkZ;
    int dx, dz, dist;
    TerrainGen Terrain;

public:
    std::mutex GenMutex;
    std::mutex ResultMutex;
    std::condition_variable GenCV;
    bool Running = true;
    std::queue<std::pair<int, int>> GenQueue;
    std::deque<Chunk> ReadyChunks;
    std::thread WorkerThread;
    std::unordered_set<std::pair<int,int>, World_Map::pair_hash> GeneratingChunks;

    ChunkGeneration(int seed, float basefreq, float baseamp, int oct, float addfreq, float addamp, float biomefreq, float biomemult, float biomebase, float biomepower);

    void LookForChunks();
    void GenerateChunk(glm::ivec3 ChunkSize);

    void Start(const glm::ivec3& ChunkSize) {
        WorkerThread = std::thread(&ChunkGeneration::GenerateChunk, this, ChunkSize);
        std::cout << "Created Generation Thread\n";
    }

    void Stop() {
        Running = false;
        GenCV.notify_all();
        if (WorkerThread.joinable()) {
            WorkerThread.join();
            std::cout << "Stopped Generation Thread\n";
        }
    }

    static void RemoveChunks();
};