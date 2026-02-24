#pragma once
#include <array>
#include <glm/glm.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "Chunk.hpp"
#include "Frustum.hpp"
#include "World/Chunk.hpp"


namespace Meshing {
    static constexpr int SIZE = 16*256*16;

    struct ChunkSnapshot {
        glm::ivec2 ChunkPos;
        std::array<Chunk::Block, SIZE> blocks;
        std::array<std::array<Chunk::Block, SIZE>, 4> neighbor;
    };

    class Mesher {
    private:
        std::mutex QueueMutex;
        std::mutex FQueueMutex;

        std::condition_variable MeshCV;
        std::atomic<bool> Running{false};
        std::vector<std::thread> Workers;

        std::queue<ChunkSnapshot> Queue;
        std::deque<std::vector<Chunk::Vertex>> FQueue;

        void Meshing_Worker();
    public:
        void Manager(const Frustum::Frust &Frust, glm::ivec3 Chunk_Size);

        void Create_Workers(const int threads) {
            Running = true;
            for (int n = 0; n < threads; ++n) {
                Workers.emplace_back(&Meshing_Worker, this);
            }
            std::cout << "Created #" << threads << " Meshing Workers\n";
        }

        void Stop_Workers() {
            Running = false;
            for (auto& worker : Workers) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
            std::cout << "Stopped #" << Workers.size() << " Meshing Workers\n";
            Workers.clear();
        }
    };

}