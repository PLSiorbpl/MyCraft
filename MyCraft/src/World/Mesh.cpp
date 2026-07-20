#include "Mesh.hpp"

#include <array>

#include "World.hpp"

void Mesh::GenerateMesh(Chunk& chunk) {
    const int chunkX = chunk.chunkX;
    const int chunkZ = chunk.chunkZ;
    const int worldOffsetX = chunkX * Chunk::WIDTH;
    const int worldOffsetZ = chunkZ * Chunk::DEPTH;

    const auto cxp = World_Map::find_chunk(chunkX + 1, chunkZ);
    const auto cxn = World_Map::find_chunk(chunkX - 1, chunkZ);
    const auto czp = World_Map::find_chunk(chunkX, chunkZ + 1);
    const auto czn = World_Map::find_chunk(chunkX, chunkZ - 1);

    auto *vertices = &chunk.Mesh;

    std::vector<uint32_t> Visible;
    Visible.resize(static_cast<size_t>(Chunk::HEIGHT) * static_cast<size_t>(Chunk::DEPTH));

    auto VisibleAt = [&](int y, int z) -> uint32_t& {
        return Visible[static_cast<size_t>(y) * static_cast<size_t>(Chunk::DEPTH) + static_cast<size_t>(z)];
    };

    // Visible Blocks
    for (int y = 0; y < Chunk::HEIGHT; y++) {
        for (int z = 0; z < Chunk::DEPTH; z++) {
            uint32_t bits = 0;
            for (int x = 0; x < Chunk::WIDTH; x++) {
                if (chunk.get_state(x, y, z)->is_solid) {
                    bits |= (static_cast<uint32_t>(1) << x);
                }
            }
            VisibleAt(y, z) = bits;
        }
    }

    // Visible Faces
    for (int y = 0; y < Chunk::HEIGHT; y++) {
        for (int z = 0; z < Chunk::DEPTH; z++) {
            const uint32_t current = VisibleAt(y, z);
            //-------------------------
            // Z+
            uint32_t next;
            if (z + 1 < Chunk::DEPTH) {
                next = VisibleAt(y, z+1);
            } else {
                uint32_t bits = 0;
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    if (czp) {
                        if (czp->get_state(x, y, 0)->is_solid)
                            bits |= (static_cast<uint32_t>(1) << x);
                    } else
                        bits |= (static_cast<uint32_t>(1) << x);
                }
                next = bits;
            }
            const uint32_t visibleZp = current & ~next;
            //-------------------------
            // Z-
            uint32_t prev;
            if (z - 1 >= 0) {
                prev = VisibleAt(y, z-1);
            } else {
                uint32_t bits = 0;
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    if (czn) {
                        if (czn->get_state(x, y, Chunk::DEPTH - 1)->is_solid)
                            bits |= (static_cast<uint32_t>(1) << x);
                    } else
                        bits |= (static_cast<uint32_t>(1) << x);
                }
                prev = bits;
            }
            const uint32_t visibleZm = current & ~prev;
            //-------------------------
            // Y+
            uint32_t nextY;
            if (y + 1  < Chunk::HEIGHT) {
                nextY = VisibleAt(y+1, z);
            } else {
                uint32_t bits = 0;
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    if (y+1 < Chunk::HEIGHT) {
                        if (IsBlockAt(worldOffsetX + x, y + 1, worldOffsetZ + z)) {
                            bits |= (static_cast<uint32_t>(1) << x);
                        }
                    }
                }
                nextY = bits;
            }
            const uint32_t visibleYp = current & ~nextY;
            //-------------------------
            // Y-
            const uint32_t prevY = (y > 0) ? VisibleAt(y-1, z) : 0ULL;
            const uint32_t visibleYm = current & ~prevY;

            //-------------------------
            // Meshing
            for (int x = 0; x < Chunk::WIDTH; x++) {
                auto block = chunk.get_state(x, y, z);
                if (!block->is_solid) continue;
                const glm::vec3 w = {worldOffsetX + x, static_cast<float>(y), worldOffsetZ + z};

                const glm::ivec2 &tex = block->uv;

                const uint32_t mask = (static_cast<uint32_t>(1) << x);
                if (visibleZp & mask) MeshZFace(vertices, w, 1, tex,  1);
                if (visibleZm & mask) MeshZFace(vertices, w, 1, tex, -1);
                if (visibleYp & mask) MeshYFace(vertices, w, 1, tex,  1);
                if (visibleYm & mask) MeshYFace(vertices, w, 1, tex, -1);
                //-------------------------
                // X+
                if (x + 1 < Chunk::WIDTH) {
                    if (!chunk.get_state(x+1, y, z)->is_solid)
                        MeshXFace(vertices, w, 1, tex, 1);
                } else if (cxp && !cxp->get_state(0, y, z)->is_solid) {
                    MeshXFace(vertices, w, 1, tex, 1);
                }
                //-------------------------
                // X-
                if (x - 1 >= 0) {
                    if (!chunk.get_state(x-1, y, z)->is_solid)
                        MeshXFace(vertices, w, 1, tex, -1);
                } else if (cxn && !cxn->get_state(Chunk::WIDTH - 1, y, z)->is_solid) {
                    MeshXFace(vertices, w, 1, tex, -1);
                }
            }
        }
    }
    Visible.clear();
    Visible.shrink_to_fit();
}
// 0 - Top | 1 - Right | 2 - Left | 3 - Bottom
inline void getUVs(std::array<glm::vec2, 4>& outUV, const glm::ivec2& BaseCoord, const float Side) {
    constexpr float tileSize = 1.0f / 8.0f; // 0.125 8 textures in a row

    const float u = BaseCoord.x * tileSize;
    const float v = (BaseCoord.y + Side) * tileSize;

    const float u1 = u + tileSize;
    const float v1 = v + tileSize;

    outUV[0] = {u,  v1};
    outUV[1] = {u,  v};
    outUV[2] = {u1, v1};
    outUV[3] = {u1, v};
};

void Mesh::MeshXFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    constexpr float size = 1.0f;
    const float z1 = w.z;
    const float y1 = w.y;
    const float z2 = w.z + width * size;
    const float y2 = w.y + size;

    const float x = w.x + (dir > 0 ? size : 0.0f); // X+ lub X-

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, texCoord, 1);

    auto push = [&](float x, float y, float z, float u, float v) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint8_t>(u*255.0f);
        ver.uv[1] = static_cast<uint8_t>(v*255.0f);
        ver.normal = (dir > 0 ? 1 : 0);
        vertices->push_back(ver);
    };

    // First Triangle
    push(x, y1, z1, uv[0].x, uv[0].y);
    push(x, y1, z2, uv[2].x, uv[2].y);
    push(x, y2, z2, uv[3].x, uv[3].y);

    // Second Triangle
    push(x, y1, z1, uv[0].x, uv[0].y);
    push(x, y2, z2, uv[3].x, uv[3].y);
    push(x, y2, z1, uv[1].x, uv[1].y);
}

void Mesh::MeshYFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    constexpr float size = 1.0f;
    const float x1 = w.x;
    const float z1 = w.z;
    const float x2 = w.x + width * size;
    const float z2 = w.z + size;

    const float y = w.y + (dir > 0 ? size : 0.0f);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, texCoord, (dir > 0 ? 0 : 3));

    auto push = [&](float x, float y, float z, float u, float v) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint8_t>(u*255.0f);
        ver.uv[1] = static_cast<uint8_t>(v*255.0f);
        ver.normal = (dir > 0 ? 3 : 2);
        vertices->push_back(ver);
    };

    // First Triangle
    push(x1, y, z1, uv[0].x, uv[0].y);
    push(x2, y, z1, uv[2].x, uv[2].y);
    push(x2, y, z2, uv[3].x, uv[3].y);

    // Second Triangle
    push(x1, y, z1, uv[0].x, uv[0].y);
    push(x2, y, z2, uv[3].x, uv[3].y);
    push(x1, y, z2, uv[1].x, uv[1].y);
}

void Mesh::MeshZFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    constexpr float size = 1.0f;
    const float x1 = w.x;
    const float y1 = w.y;
    const float x2 = w.x + width * size;
    const float y2 = w.y + size;

    const float z = w.z + (dir > 0 ? size : 0.0f);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, texCoord, 2);

    auto push = [&](float x, float y, float z, float u, float v) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint8_t>(u*255.0f);
        ver.uv[1] = static_cast<uint8_t>(v*255.0f);
        ver.normal = (dir > 0 ? 5 : 4);
        vertices->push_back(ver);
    };

    // First Triangle
    push(x1, y1, z, uv[0].x, uv[0].y);
    push(x2, y1, z, uv[2].x, uv[2].y);
    push(x2, y2, z, uv[3].x, uv[3].y);

    // Second Triangle
    push(x1, y1, z, uv[0].x, uv[0].y);
    push(x2, y2, z, uv[3].x, uv[3].y);
    push(x1, y2, z, uv[1].x, uv[1].y);
}

inline bool Mesh::IsBlockAt(const int WorldX, int y, const int WorldZ) {
    const int chunkX = (WorldX >= 0) ? WorldX / Chunk::WIDTH : (WorldX - Chunk::WIDTH + 1) / Chunk::WIDTH;
    const int chunkZ = (WorldZ >= 0) ? WorldZ / Chunk::DEPTH : (WorldZ - Chunk::DEPTH + 1) / Chunk::DEPTH;

    const int localX = WorldX - chunkX * Chunk::WIDTH;
    const int localZ = WorldZ - chunkZ * Chunk::DEPTH;

    const auto it = World_Map::World.find({chunkX, chunkZ});
    if (it == World_Map::World.end()) return true;

    const Chunk& chunk = it->second;
    return chunk.get_state(localX, y, localZ)->is_solid;
}
