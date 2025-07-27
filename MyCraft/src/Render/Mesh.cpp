#include "Mesh.hpp"

void Mesh::GenerateMesh(const Chunk& chunk, std::vector<float>& vertices, int chunkX, int chunkZ, glm::ivec3 ChunkSize, int RenderDist, std::map<std::pair<int, int>, Chunk> &World) {
    // Pre Allocation
    int maxChunks = (RenderDist * 2 + 1) * (RenderDist * 2 + 1);
    int maxBlocks = ChunkSize.x * ChunkSize.y * ChunkSize.z;
    vertices.reserve((maxBlocks * maxChunks) * 36);

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

                    CubeMesh(vertices, glm::vec3(wx, wy, wz), chunk, glm::ivec3(x, y, z), ChunkSize, World);
                }
            }
        }
    }
}

void Mesh::CubeMesh(std::vector<float>& vertices, glm::vec3 w, const Chunk& chunk, glm::ivec3 Local, glm::ivec3 ChunkSize, std::map<std::pair<int, int>, Chunk> &World) {
    float size = 1.0f;

    glm::vec3 p000 = {w.x,      w.y,      w.z};
    glm::vec3 p001 = {w.x,      w.y,      w.z+size};
    glm::vec3 p010 = {w.x,      w.y+size, w.z};
    glm::vec3 p011 = {w.x,      w.y+size, w.z+size};
    glm::vec3 p100 = {w.x+size, w.y,      w.z};
    glm::vec3 p101 = {w.x+size, w.y,      w.z+size};
    glm::vec3 p110 = {w.x+size, w.y+size, w.z};
    glm::vec3 p111 = {w.x+size, w.y+size, w.z+size};

    int texX = 0;
    int texY = 0;

    const Chunk::Block& block = chunk.get(Local.x, Local.y, Local.z);
    if (block.id == 1) {
        texX = 0;
        texY = 0;
    } else if (block.id == 2) {
        texX = 1;
        texY = 0;
    } else {
        texX = 0;
        texY = 1;
    }

    const float tileSize = 1.0f / 8.0f; // 0.125   8 textures in a row

    float u = texX * tileSize;
    float v = texY * tileSize;

    glm::vec2 uv00 = {u,  v};                      // left down
    glm::vec2 uv10 = {u + tileSize, v};            // right up
    glm::vec2 uv01 = {u,  v + tileSize};           // left up
    glm::vec2 uv11 = {u + tileSize, v + tileSize}; // right down

    auto pushTri = [&](glm::vec3 a, glm::vec2 uva,
                       glm::vec3 b, glm::vec2 uvb,
                       glm::vec3 c, glm::vec2 uvc) {
        for (int i = 0; i < 3; ++i) {
            const glm::vec3& v = i == 0 ? a : (i == 1 ? b : c);
            const glm::vec2& uv = i == 0 ? uva : (i == 1 ? uvb : uvc);
            vertices.insert(vertices.end(), { v.x, v.y, v.z, uv.x, uv.y });
        }
    };

    // FRONT (z+)
    if ((Local.z + 1 >= ChunkSize.z)) {
        if (!IsBlockAt(World, w.x, w.y, w.z+1, ChunkSize)) {
            pushTri(p001, uv00, p101, uv10, p111, uv11);
            pushTri(p001, uv00, p111, uv11, p011, uv01);
        }
    } else if (chunk.get(Local.x, Local.y, Local.z+1).id == 0) {
        pushTri(p001, uv00, p101, uv10, p111, uv11);
        pushTri(p001, uv00, p111, uv11, p011, uv01);
    }

    // BACK (z-)
    if ((Local.z - 1 < 0)) {
        if (!IsBlockAt(World, w.x, w.y, w.z-1, ChunkSize)) {
            pushTri(p100, uv00, p000, uv10, p010, uv11);
            pushTri(p100, uv00, p010, uv11, p110, uv01);
        }
    } else if (chunk.get(Local.x, Local.y, Local.z-1).id == 0) {
        pushTri(p100, uv00, p000, uv10, p010, uv11);
        pushTri(p100, uv00, p010, uv11, p110, uv01);
    }

    // LEFT (x-)
    if ((Local.x - 1 < 0)) {
        if (!IsBlockAt(World, w.x-1, w.y, w.z, ChunkSize)) {
            pushTri(p000, uv00, p001, uv10, p011, uv11);
            pushTri(p000, uv00, p011, uv11, p010, uv01);
        }
    } else if (chunk.get(Local.x-1, Local.y, Local.z).id == 0) {
        pushTri(p000, uv00, p001, uv10, p011, uv11);
        pushTri(p000, uv00, p011, uv11, p010, uv01);
    }

    // RIGHT (x+)
    if ((Local.x + 1 >= ChunkSize.x)) {
        if (!IsBlockAt(World, w.x+1, w.y, w.z, ChunkSize)) {
            pushTri(p100, uv00, p101, uv10, p111, uv11);
            pushTri(p100, uv00, p111, uv11, p110, uv01);
        }
    } else if (chunk.get(Local.x+1, Local.y, Local.z).id == 0) {
        pushTri(p100, uv00, p101, uv10, p111, uv11);
        pushTri(p100, uv00, p111, uv11, p110, uv01);
    }

    // TOP (y+)
    if (Local.y + 1 >= ChunkSize.y || chunk.get(Local.x, Local.y+1, Local.z).id == 0) {
        pushTri(p010, uv00, p011, uv10, p111, uv11);
        pushTri(p010, uv00, p111, uv11, p110, uv01);
    }

    // BOTTOM (y-)
    if ((Local.y - 1 < 0) || chunk.get(Local.x, Local.y-1, Local.z).id == 0) {
        pushTri(p000, uv00, p100, uv10, p101, uv11);
        pushTri(p000, uv00, p101, uv11, p001, uv01);
    }
}

bool Mesh::IsBlockAt(std::map<std::pair<int, int>, Chunk> &World, int WorldX, int y, int WorldZ, glm::ivec3 ChunkSize) {
    int chunkX = WorldX / ChunkSize.x;
    int chunkZ = WorldZ / ChunkSize.z;
    if (WorldX < 0 && WorldX % ChunkSize.x != 0) chunkX--;
    if (WorldZ < 0 && WorldZ % ChunkSize.z != 0) chunkZ--;

    int localX = WorldX - chunkX * ChunkSize.x;
    int localZ = WorldZ - chunkZ * ChunkSize.z;

    auto it = World.find({chunkX, chunkZ});
    if (it != World.end()) {
        const Chunk& neighbor = it->second;
        return neighbor.get(localX, y, localZ).id != 0;
    }
    return false;
}
