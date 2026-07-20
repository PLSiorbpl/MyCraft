#include "World/World.hpp"

#include "Block.hpp"

std::unordered_map<std::pair<int, int>, Chunk, World_Map::PairHash> World_Map::World;
std::vector<World_Map::Render_Info> World_Map::Render_List;
std::vector<Chunk*> World_Map::Mesh_Queue;
