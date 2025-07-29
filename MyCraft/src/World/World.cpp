#include "World/World.hpp"

std::unordered_map<std::pair<int, int>, Chunk, World_Map::pair_hash> World_Map::World;
