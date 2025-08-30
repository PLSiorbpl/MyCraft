#include "Mesh.hpp"

void Mesh::GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, const int chunkX, const int chunkZ, const glm::ivec3 ChunkSize, const int RenderDist) {
    const int worldOffsetX = chunkX * ChunkSize.x;
    const int worldOffsetZ = chunkZ * ChunkSize.z;

    for (int x = 0; x < ChunkSize.x; ++x) {
        for (int y = 0; y < ChunkSize.y; ++y) {
            for (int z = 0; z < ChunkSize.z; ++z) {
                const Chunk::Block& block = chunk.get(x, y, z);
                if (block.id != 0) { // 0 is Air
                    const float wx = worldOffsetX + x;
                    const float wy = y;
                    const float wz = worldOffsetZ + z;

                    CubeMesh(vertices, glm::vec3(wx, wy, wz), chunk, glm::ivec3(x, y, z), ChunkSize);
                }
            }
        }
    }
}
// 0 - Top | 1 - Right | 2 - Left | 3 - Bottom
auto getUVs = [](const glm::ivec2 BaseCoord, const float Side) {
    static const float tileSize = 1.0f / 8.0f; // 0.125 8 textures in a row

    const float u = BaseCoord.x * tileSize;
    const float v = (BaseCoord.y + Side) * tileSize;

    const glm::vec2 uv00 = {u,  v + tileSize};
    const glm::vec2 uv01 = {u + tileSize, v + tileSize};
    const glm::vec2 uv10 = {u,  v};
    const glm::vec2 uv11 = {u + tileSize, v};

    return std::array<glm::vec2, 4>{uv00, uv10, uv01, uv11};
};

void Mesh::CubeMesh(std::vector<float>& vertices, const glm::vec3 w, const Chunk& chunk, const glm::ivec3 Local, const glm::ivec3 ChunkSize) {
    const float size = 1.0f;

    const glm::vec3 p000 = {w.x,      w.y,      w.z     };
    const glm::vec3 p001 = {w.x,      w.y,      w.z+size};
    const glm::vec3 p010 = {w.x,      w.y+size, w.z     };
    const glm::vec3 p011 = {w.x,      w.y+size, w.z+size};
    const glm::vec3 p100 = {w.x+size, w.y,      w.z     };
    const glm::vec3 p101 = {w.x+size, w.y,      w.z+size};
    const glm::vec3 p110 = {w.x+size, w.y+size, w.z     };
    const glm::vec3 p111 = {w.x+size, w.y+size, w.z+size};

    const Chunk::Block& block = chunk.get(Local.x, Local.y, Local.z);

    // UV mapping
    glm::ivec2 texCoord(0, 0);
    switch (block.id) {
        case 1: texCoord = {0, 0}; break; // stone
        case 2: texCoord = {1, 0}; break; // grass
        case 3: texCoord = {2, 0}; break; // dirt
        case 4: texCoord = {3, 0}; break; // Blacha
        case 5: texCoord = {4, 0}; break; // Wool
        default: texCoord = {4, 4}; break; // Nothing
    }
    
    auto pushTri = [&](const glm::vec3& a, const glm::vec2& uva,
                   const glm::vec3& b, const glm::vec2& uvb,
                   const glm::vec3& c, const glm::vec2& uvc,
                   const glm::vec3& n) {
        const glm::vec3 pts[3] = {a,b,c};
        const glm::vec2 uvs[3] = {uva,uvb,uvc};
        for (int i = 0; i < 3; ++i) {
            vertices.push_back(pts[i].x);
            vertices.push_back(pts[i].y);
            vertices.push_back(pts[i].z);
            vertices.push_back(uvs[i].x);
            vertices.push_back(uvs[i].y);
            vertices.push_back(n.x);
            vertices.push_back(n.y);
            vertices.push_back(n.z);
        }
    };

    // FRONT (z+)
    if ((Local.z + 1 >= ChunkSize.z)) {
        if (!IsBlockAt(w.x, w.y, w.z+1, ChunkSize)) {
            const auto uv = getUVs(texCoord, 1);
            pushTri(p001, uv[0], p101, uv[2], p111, uv[3], glm::vec3(0, 0, 1));
            pushTri(p001, uv[0], p111, uv[3], p011, uv[1], glm::vec3(0, 0, 1));
        }
    } else if (chunk.get(Local.x, Local.y, Local.z+1).id == 0) {
        const auto uv = getUVs(texCoord, 1);
        pushTri(p001, uv[0], p101, uv[2], p111, uv[3], glm::vec3(0, 0, 1));
        pushTri(p001, uv[0], p111, uv[3], p011, uv[1], glm::vec3(0, 0, 1));
    }

    // BACK (z-)
    if ((Local.z - 1 < 0)) {
        const auto uv = getUVs(texCoord, 1);
        if (!IsBlockAt(w.x, w.y, w.z-1, ChunkSize)) {
            pushTri(p100, uv[0], p000, uv[2], p010, uv[3], glm::vec3(0, 0, -1));
            pushTri(p100, uv[0], p010, uv[3], p110, uv[1], glm::vec3(0, 0, -1));
        }
    } else if (chunk.get(Local.x, Local.y, Local.z-1).id == 0) {
        const auto uv = getUVs(texCoord, 1);
        pushTri(p100, uv[0], p000, uv[2], p010, uv[3], glm::vec3(0, 0, -1));
        pushTri(p100, uv[0], p010, uv[3], p110, uv[1], glm::vec3(0, 0, -1));
    }

    // LEFT (x-)
    if ((Local.x - 1 < 0)) {
        const auto uv = getUVs(texCoord, 1);
        if (!IsBlockAt(w.x-1, w.y, w.z, ChunkSize)) {
            pushTri(p000, uv[0], p001, uv[2], p011, uv[3], glm::vec3(-1, 0, 0));
            pushTri(p000, uv[0], p011, uv[3], p010, uv[1], glm::vec3(-1, 0, 0));
        }
    } else if (chunk.get(Local.x-1, Local.y, Local.z).id == 0) {
        const auto uv = getUVs(texCoord, 1);
        pushTri(p000, uv[0], p001, uv[2], p011, uv[3], glm::vec3(-1, 0, 0));
        pushTri(p000, uv[0], p011, uv[3], p010, uv[1], glm::vec3(-1, 0, 0));
    }

    // RIGHT (x+)
    if ((Local.x + 1 >= ChunkSize.x)) {
        const auto uv = getUVs(texCoord, 1);
        if (!IsBlockAt(w.x+1, w.y, w.z, ChunkSize)) {
            pushTri(p100, uv[0], p101, uv[2], p111, uv[3], glm::vec3(1, 0, 0));
            pushTri(p100, uv[0], p111, uv[3], p110, uv[1], glm::vec3(1, 0, 0));
        }
    } else if (chunk.get(Local.x+1, Local.y, Local.z).id == 0) {
        const auto uv = getUVs(texCoord, 1);
        pushTri(p100, uv[0], p101, uv[2], p111, uv[3], glm::vec3(1, 0, 0));
        pushTri(p100, uv[0], p111, uv[3], p110, uv[1], glm::vec3(1, 0, 0));
    }

    // TOP (y+)
    if (Local.y + 1 >= ChunkSize.y || chunk.get(Local.x, Local.y+1, Local.z).id == 0) {
        const auto uv = getUVs(texCoord, 0);
        pushTri(p010, uv[0], p011, uv[2], p111, uv[3], glm::vec3(0, 1, 0));
        pushTri(p010, uv[0], p111, uv[3], p110, uv[1], glm::vec3(0, 1, 0));
    }

    // BOTTOM (y-)
    if ((Local.y - 1 < 0) || chunk.get(Local.x, Local.y-1, Local.z).id == 0) {
        const auto uv = getUVs(texCoord, 0);
        pushTri(p000, uv[0], p100, uv[2], p101, uv[3], glm::vec3(0, -1, 0));
        pushTri(p000, uv[0], p101, uv[3], p001, uv[1], glm::vec3(0, -1, 0));
    }
}

bool Mesh::IsBlockAt(int WorldX, int y, int WorldZ, const glm::ivec3 ChunkSize) {
    const auto& World = World_Map::World;
    const int chunkX = std::floor((float)WorldX / ChunkSize.x);
    const int chunkZ = std::floor((float)WorldZ / ChunkSize.z);

    const int localX = WorldX - chunkX * ChunkSize.x;
    const int localZ = WorldZ - chunkZ * ChunkSize.z;

    const auto it = World.find({chunkX, chunkZ});
    if (it != World.end()) {
        const Chunk& neighbor = it->second;
        return neighbor.get(localX, y, localZ).id != 0;
    }
    return true;
}
