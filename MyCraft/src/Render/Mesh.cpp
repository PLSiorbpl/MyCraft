#include "Mesh.hpp"
#include <iostream>
static const glm::ivec2 BlockUVs[] = {
    {0,0}, // 0 - air
    {0,0}, // 1 - stone
    {1,0}, // 2 - grass
    {2,0}, // 3 - dirt
    {3,0}, // 4 - Blacha
    {4,0}, // 5 - Wool
    {5,0}  // 6 - Water
};

void Mesh::GenerateMesh(const Chunk& chunk, std::vector<Chunk::Vertex>& vertices, const int chunkX, const int chunkZ, const glm::ivec3 ChunkSize, const int RenderDist) {
    const int worldOffsetX = chunkX * ChunkSize.x;
    const int worldOffsetZ = chunkZ * ChunkSize.z;
    uint16_t Visible[ChunkSize.y][ChunkSize.z]; // x is uint16_t

    // Visible Blocks
    for (int y = 0; y < ChunkSize.y; y++) {
        for (int z = 0; z < ChunkSize.z; z++) {
            uint16_t bits = 0;
            for (int x = 0; x < ChunkSize.x; x++) {
                const bool vis = chunk.get(x, y, z).solid;
                if (vis) {
                    bits |= (1 << x);
                }
            }
            Visible[y][z] = bits;
        }
    }

    // Visible Faces
    for (int y = 0; y < ChunkSize.y; y++) {
        for (int z = 0; z < ChunkSize.z; z++) {
            const uint16_t current = Visible[y][z];
            //-------------------------
            // Z+
            uint16_t next;
            if (z + 1 < ChunkSize.z) {
                next = Visible[y][z + 1];
            } else {
                uint16_t bits = 0;
                for (int x = 0; x < ChunkSize.x; x++) {
                    if (IsBlockAt(worldOffsetX + x, y, worldOffsetZ + z+1, ChunkSize)) {
                        bits |= (1 << x);
                    }
                }
                next = bits;
            }
            const uint16_t visibleZp = current & ~next;
            //-------------------------
            // Z-
            uint16_t prev;
            if (z - 1 >= 0) {
                prev = Visible[y][z - 1];
            } else {
                uint16_t bits = 0;
                for (int x = 0; x < ChunkSize.x; x++) {
                    if (IsBlockAt(worldOffsetX + x, y, worldOffsetZ + z - 1, ChunkSize)) {
                        bits |= (1 << x);
                    }
                }
                prev = bits;
            }
            const uint16_t visibleZm = current & ~prev;
            //-------------------------
            // Y+
            uint16_t nextY;
            if (y + 1  < ChunkSize.y) {
                nextY = Visible[y + 1][z];
            } else {
                uint16_t bits = 0;
                for (int x = 0; x < ChunkSize.x; x++) {
                    if (IsBlockAt(worldOffsetX + x, y + 1, worldOffsetZ + z, ChunkSize)) {
                        bits |= (1 << x);
                    }
                }
                nextY = bits;
            }
            const uint16_t visibleYp = current & ~nextY;
            //-------------------------
            // Y-
            const uint16_t prevY = (y > 0) ? Visible[y - 1][z] : 0;
            const uint16_t visibleYm = current & ~prevY;

            //-------------------------
            // Meshing
            for (int x = 0; x < ChunkSize.x; x++) {
                const auto block = chunk.get(x, y, z);
                if (!block.solid) continue;
                const glm::vec3 w = {worldOffsetX + x, float(y), worldOffsetZ + z};

                const glm::ivec2 tex = BlockUVs[block.id];

                if (visibleZp & (1 << x)) MeshZFace(vertices, w, 1, tex,  1);
                if (visibleZm & (1 << x)) MeshZFace(vertices, w, 1, tex, -1);
                if (visibleYp & (1 << x)) MeshYFace(vertices, w, 1, tex,  1);
                if (visibleYm & (1 << x)) MeshYFace(vertices, w, 1, tex, -1);
                //-------------------------
                // X+
                if ((x + 1 < ChunkSize.x && !chunk.get(x + 1, y, z).solid) ||
                    (x + 1 >= ChunkSize.x && !IsBlockAt(worldOffsetX + x + 1, y, worldOffsetZ + z, ChunkSize)))
                    MeshXFace(vertices, w, 1, tex, 1);
                //-------------------------
                // X-
                if ((x > 0 && !chunk.get(x - 1, y, z).solid) ||
                    (x == 0 && !IsBlockAt(worldOffsetX + x - 1, y, worldOffsetZ + z, ChunkSize)))
                    MeshXFace(vertices, w, 1, tex, -1);
            }
        }
    }
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

void Mesh::MeshXFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    const float size = 1.0f;
    const float z1 = w.z;
    const float y1 = w.y;
    const float z2 = w.z + width * size;
    const float y2 = w.y + size;

    const float x = w.x + (dir > 0 ? size : 0.0f); // X+ lub X-

    const glm::vec3 normal(static_cast<float>(dir), 0, 0);
    std::array<glm::vec2, 4> uv;
    getUVs(uv, texCoord, 1);

    auto push = [&](float x, float y, float z, float u, float v) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint8_t>(u*255.0f);
        ver.uv[1] = static_cast<uint8_t>(v*255.0f);
        ver.normal = (dir > 0 ? 1 : 0);
        vertices.push_back(ver);
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

void Mesh::MeshYFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    const float size = 1.0f;
    const float x1 = w.x;
    const float z1 = w.z;
    const float x2 = w.x + width * size;
    const float z2 = w.z + size;

    const float y = w.y + (dir > 0 ? size : 0.0f);

    std::array<glm::vec2, 4> uv;
    getUVs(uv, texCoord, (dir > 0 ? 0 : 3));

    auto push = [&](float x, float y, float z, float u, float v) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint8_t>(u*255.0f);
        ver.uv[1] = static_cast<uint8_t>(v*255.0f);
        ver.normal = (dir > 0 ? 3 : 2);
        vertices.push_back(ver);
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

void Mesh::MeshZFace(std::vector<Chunk::Vertex>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    const float size = 1.0f;
    const float x1 = w.x;
    const float y1 = w.y;
    const float x2 = w.x + width * size;
    const float y2 = w.y + size;

    const float z = w.z + (dir > 0 ? size : 0.0f);

    std::array<glm::vec2, 4> uv;
    getUVs(uv, texCoord, 2);

    auto push = [&](float x, float y, float z, float u, float v) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint8_t>(u*255.0f);
        ver.uv[1] = static_cast<uint8_t>(v*255.0f);
        ver.normal = (dir > 0 ? 5 : 4);
        vertices.push_back(ver);
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

inline bool Mesh::IsBlockAt(int WorldX, int y, int WorldZ, const glm::ivec3& ChunkSize) {
    // użyj dzielenia całkowitego zamiast floata
    const int chunkX = (WorldX >= 0) ? WorldX / ChunkSize.x : (WorldX - ChunkSize.x + 1) / ChunkSize.x;
    const int chunkZ = (WorldZ >= 0) ? WorldZ / ChunkSize.z : (WorldZ - ChunkSize.z + 1) / ChunkSize.z;

    const int localX = WorldX - chunkX * ChunkSize.x;
    const int localZ = WorldZ - chunkZ * ChunkSize.z;

    const auto it = World_Map::World.find({chunkX, chunkZ});
    if (it == World_Map::World.end()) return true; // poza światem traktuj jako "blok"

    const Chunk& chunk = it->second;
    return chunk.get(localX, y, localZ).id != 0;
}
