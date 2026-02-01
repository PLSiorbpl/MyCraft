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
    std::atomic<bool> Running{false};
    std::queue<std::pair<int, int>> GenQueue;
    std::deque<Chunk> ReadyChunks;
    std::vector<std::thread> Workers;
    std::unordered_set<std::pair<int,int>, World_Map::pair_hash> GeneratingChunks;

    void LookForChunks();
    void GenerateChunk(glm::ivec3 ChunkSize);

    void Start(const int Threads, const glm::ivec3& ChunkSize) {
        Running = true;
        for (int n = 0; n < Threads; ++n) {
            Workers.emplace_back(
                &ChunkGeneration::GenerateChunk,
                this,
                ChunkSize
            );
        }
        std::cout << "Created " << Threads << " Generation Threads\n";
    }

    void Stop() {
        Running = false;
        GenCV.notify_all();
        for (auto& worker : Workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        std::cout << "Stopped " << Workers.size() << " Generation Threads\n";
        Workers.clear();
    }

    static void RemoveChunks();
};