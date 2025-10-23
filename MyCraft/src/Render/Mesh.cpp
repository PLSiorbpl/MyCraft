#include "Mesh.hpp"

static const glm::ivec2 BlockUVs[] = {
    {0,0}, // 0 - air
    {0,0}, // 1 - stone
    {1,0}, // 2 - grass
    {2,0}, // 3 - dirt
    {3,0}, // 4 - Blacha
    {4,0}, // 5 - Wool
    {5,0}  // 6 - Water
};

void Mesh::GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, const int chunkX, const int chunkZ, const glm::ivec3 ChunkSize, const int RenderDist) {
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

void Mesh::MeshXFace(std::vector<float>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
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
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(u);
        vertices.push_back(v);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
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

void Mesh::MeshYFace(std::vector<float>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    const float size = 1.0f;
    const float x1 = w.x;
    const float z1 = w.z;
    const float x2 = w.x + width * size;
    const float z2 = w.z + size;

    const float y = w.y + (dir > 0 ? size : 0.0f);

    const glm::vec3 normal(0, static_cast<float>(dir), 0);
    std::array<glm::vec2, 4> uv;
    getUVs(uv, texCoord, (dir > 0 ? 0 : 3));

    auto push = [&](float x, float y, float z, float u, float v) {
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(u);
        vertices.push_back(v);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
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

void Mesh::MeshZFace(std::vector<float>& vertices, const glm::vec3& w, const int width, const glm::ivec2& texCoord, const int dir) {
    const float size = 1.0f;
    const float x1 = w.x;
    const float y1 = w.y;
    const float x2 = w.x + width * size;
    const float y2 = w.y + size;

    const float z = w.z + (dir > 0 ? size : 0.0f);

    const glm::vec3 normal(0, 0, static_cast<float>(dir));
    std::array<glm::vec2, 4> uv;
    getUVs(uv, texCoord, 2);

    auto push = [&](float x, float y, float z, float u, float v) {
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(u);
        vertices.push_back(v);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
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

void Mesh::CubeMesh(std::vector<float>& vertices, const glm::vec3 w, const Chunk& chunk, const glm::ivec3 Local, const glm::ivec3 ChunkSize) {
    const float size = 1.0f;
    float ao = 0.0f;

    glm::vec3 p000 = {w.x,      w.y,      w.z     };
    glm::vec3 p001 = {w.x,      w.y,      w.z+size};
    glm::vec3 p010 = {w.x,      w.y+size, w.z     };
    glm::vec3 p011 = {w.x,      w.y+size, w.z+size};
    glm::vec3 p100 = {w.x+size, w.y,      w.z     };
    glm::vec3 p101 = {w.x+size, w.y,      w.z+size};
    glm::vec3 p110 = {w.x+size, w.y+size, w.z     };
    glm::vec3 p111 = {w.x+size, w.y+size, w.z+size};

    const Chunk::Block& block = chunk.get(Local.x, Local.y, Local.z);

    // UV mapping
    const glm::ivec2 texCoord = BlockUVs[block.id];
    
    auto pushTri = [&](const glm::vec3& a, const glm::vec2& uva,
                   const glm::vec3& b, const glm::vec2& uvb,
                   const glm::vec3& c, const glm::vec2& uvc,
                   const glm::vec3& n) {
        const glm::vec3 pts[3] = {a,b,c};
        const glm::vec2 uvs[3] = {uva,uvb,uvc};
        //const glm::vec3 vcolor[3] = {glm::vec3(vc.x), glm::vec3(vc.y), glm::vec3(vc.z)};
        for (int i = 0; i < 3; ++i) {
            vertices.push_back(pts[i].x);
            vertices.push_back(pts[i].y);
            vertices.push_back(pts[i].z);
            vertices.push_back(uvs[i].x);
            vertices.push_back(uvs[i].y);
            vertices.push_back(n.x);
            vertices.push_back(n.y);
            vertices.push_back(n.z);
            //vertices.push_back(vcolor[i].x);
            //vertices.push_back(vcolor[i].y);
            //vertices.push_back(vcolor[i].z);
        }
    };
    std::array<glm::vec2, 4> uv;
    // FRONT (z+)
    if ((Local.z + 1 >= ChunkSize.z)) {
        if (!IsBlockAt(w.x, w.y, w.z+1, ChunkSize)) {
            getUVs(uv, texCoord, 1);
            const glm::vec3 normal = glm::vec3(0, 0, 1);

            pushTri(p001, uv[0], p101, uv[2], p111, uv[3], normal);
            pushTri(p001, uv[0], p111, uv[3], p011, uv[1], normal);
        }
    } else if (chunk.get(Local.x, Local.y, Local.z+1).id == 0) {
        getUVs(uv, texCoord, 1);
        const glm::vec3 normal = glm::vec3(0, 0, 1);

        pushTri(p001, uv[0], p101, uv[2], p111, uv[3], normal);
        pushTri(p001, uv[0], p111, uv[3], p011, uv[1], normal);
    }

    // BACK (z-)
    if ((Local.z - 1 < 0)) {
        if (!IsBlockAt(w.x, w.y, w.z-1, ChunkSize)) {
            getUVs(uv, texCoord, 1);
            const glm::vec3 normal = glm::vec3(0, 0, -1);

            pushTri(p100, uv[0], p000, uv[2], p010, uv[3], normal);
            pushTri(p100, uv[0], p010, uv[3], p110, uv[1], normal);
        }
    } else if (chunk.get(Local.x, Local.y, Local.z-1).id == 0) {
        getUVs(uv, texCoord, 1);
        const glm::vec3 normal = glm::vec3(0, 0, -1);

        pushTri(p100, uv[0], p000, uv[2], p010, uv[3], normal);
        pushTri(p100, uv[0], p010, uv[3], p110, uv[1], normal);
    }

    // LEFT (x-)
    if ((Local.x - 1 < 0)) {
        if (!IsBlockAt(w.x-1, w.y, w.z, ChunkSize)) {
            getUVs(uv, texCoord, 2);
            const glm::vec3 normal = glm::vec3(-1, 0, 0);

            pushTri(p000, uv[0], p001, uv[2], p011, uv[3], normal);
            pushTri(p000, uv[0], p011, uv[3], p010, uv[1], normal);
        }
    } else if (chunk.get(Local.x-1, Local.y, Local.z).id == 0) {
        getUVs(uv, texCoord, 2);
        const glm::vec3 normal = glm::vec3(-1, 0, 0);

        pushTri(p000, uv[0], p001, uv[2], p011, uv[3], normal);
        pushTri(p000, uv[0], p011, uv[3], p010, uv[1], normal);
    }

    // RIGHT (x+)
    if ((Local.x + 1 >= ChunkSize.x)) {
        if (!IsBlockAt(w.x+1, w.y, w.z, ChunkSize)) {
            getUVs(uv, texCoord, 2);
            const glm::vec3 normal = glm::vec3(1, 0, 0);

            pushTri(p100, uv[0], p101, uv[2], p111, uv[3], normal);
            pushTri(p100, uv[0], p111, uv[3], p110, uv[1], normal);
        }
    } else if (chunk.get(Local.x+1, Local.y, Local.z).id == 0) {
        getUVs(uv, texCoord, 2);
        const glm::vec3 normal = glm::vec3(1, 0, 0);

        pushTri(p100, uv[0], p101, uv[2], p111, uv[3], normal);
        pushTri(p100, uv[0], p111, uv[3], p110, uv[1], normal);
    }

    // TOP (y+)
    if (Local.y + 1 >= ChunkSize.y || chunk.get(Local.x, Local.y+1, Local.z).id == 0) {
        getUVs(uv, texCoord, 0);
        const glm::vec3 normal = glm::vec3(0, 1, 0);

        pushTri(p010, uv[0], p011, uv[2], p111, uv[3], normal);
        pushTri(p010, uv[0], p111, uv[3], p110, uv[1], normal);
    }

    // BOTTOM (y-)
    if ((Local.y - 1 < 0) || chunk.get(Local.x, Local.y-1, Local.z).id == 0) {
        getUVs(uv, texCoord, 3);
        const glm::vec3 normal = glm::vec3(0, -1, 0);

        pushTri(p000, uv[0], p100, uv[2], p101, uv[3], normal);
        pushTri(p000, uv[0], p101, uv[3], p001, uv[1], normal);
    }
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
