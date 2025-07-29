#pragma once
#include <unordered_map>
#include "Chunk.hpp"

class World_Map {
    public:
    struct pair_hash {
        std::size_t operator()(const std::pair<int, int>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };

    static std::unordered_map<std::pair<int, int>, Chunk, pair_hash> World;
};