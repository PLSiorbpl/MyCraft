#pragma once
#include <atomic>
#include <condition_variable>
#include <format>
#include <iostream>
#include <queue>
#include <deque>
#include <vector>
#include <thread>

#include "World/World.hpp"
#include "World/Chunk.hpp"

enum class result {
    Done = 0,
    Missing_N = 1,
    Invalid_ptr = 2,
    Bad_Flags = 3
};

struct mesh_t {
    int chunkX, chunkZ;
    std::vector<Chunk::Vertex> mesh;
    result R;
};

class Mesher {
public:
    std::vector<Chunk *> pendingChunks;
    std::queue<std::pair<int, int>> meshQueue;
    std::mutex meshInMutex;
    std::deque<mesh_t> meshOutQueue;
    std::mutex meshOutMutex;
    std::condition_variable meshCV;

    ~Mesher() { stop(); }

    void start(const int threads) {
        if (running == true)
            stop();

        running = true;

        for (int i = 0; i < threads; i++)
            workers.emplace_back(&Mesher::mesh_worker, this);
        std::cout << std::format("[ MESH ] Created {} threads\n", threads);
    }

    void stop() {
        if (!running) return;

        running = false;
        meshCV.notify_all();
        for (auto& worker : workers)
            if (worker.joinable()) worker.join();
        std::cout << std::format("[ MESH ] Stopped {} threads\n", workers.size());
        workers.clear();
    }

private:
    void mesh_worker();
    void fallback(const Chunk *chunk, result r);

    std::atomic<bool> running{false};
    std::vector<std::thread> workers;
};

extern Mesher mesher;
