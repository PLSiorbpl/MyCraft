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

    std::array<std::vector<uint32_t>, 6> Occlusion;
    for (auto& v : Occlusion)
        v.resize(Chunk::HEIGHT * Chunk::DEPTH);

    // Visible Blocks
    for (int y = 0; y < Chunk::HEIGHT; y++) {
        for (int z = 0; z < Chunk::DEPTH; z++) {
            for (int x = 0; x < Chunk::WIDTH; x++) {
                const auto &b = chunk.get_state(x, y, z);
                if (!b || !b->model) continue;
                const auto &m = b->model->occlusionMask;
                const auto idx = y * Chunk::DEPTH + z;

                if (m & XP)
                    Occlusion[1][idx] |= 1u << x;
                if (m & XN)
                    Occlusion[0][idx] |= 1u << x;
                if (m & ZP)
                    Occlusion[5][idx] |= 1u << x;
                if (m & ZN)
                    Occlusion[4][idx] |= 1u << x;
                if (m & YP)
                    Occlusion[3][idx] |= 1u << x;
                if (m & YN)
                    Occlusion[2][idx] |= 1u << x;
            }
        }
    }

    // Visible Faces
    for (int y = 0; y < Chunk::HEIGHT; y++) {
        for (int z = 0; z < Chunk::DEPTH; z++) {
            const auto idx = y * Chunk::DEPTH + z;
            //-------------------------
            // Z+
            uint32_t current = Occlusion[5][idx];
            uint32_t next;
            if (z + 1 < Chunk::DEPTH) {
                const auto n_idx = y * Chunk::DEPTH + (z+1);
                next = Occlusion[4][n_idx];
            } else {
                uint32_t bits = 0;
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    if (czp) {
                        if (czp->get_state(x, y, 0)->model->occlusionMask & ZN)
                            bits |= (static_cast<uint32_t>(1) << x);
                    } else
                        bits |= (static_cast<uint32_t>(1) << x);
                }
                next = bits;
            }
            const uint32_t visibleZp = current & ~next;
            //-------------------------
            // Z-
            current = Occlusion[4][idx];
            uint32_t prev;
            if (z - 1 >= 0) {
                const auto n_idx = y * Chunk::DEPTH + z-1;
                prev = Occlusion[5][n_idx];
            } else {
                uint32_t bits = 0;
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    if (czn) {
                        if (czn->get_state(x, y, Chunk::DEPTH - 1)->model->occlusionMask & ZP)
                            bits |= (static_cast<uint32_t>(1) << x);
                    } else
                        bits |= (static_cast<uint32_t>(1) << x);
                }
                prev = bits;
            }
            const uint32_t visibleZm = current & ~prev;
            //-------------------------
            // Y+
            current = Occlusion[3][idx];
            auto n_idx = (y+1) * Chunk::DEPTH + z;
            const uint32_t nextY = (y+1 < Chunk::HEIGHT) ? Occlusion[2][n_idx] : 0;
            const uint32_t visibleYp = current & ~nextY;
            //-------------------------
            // Y-
            current = Occlusion[2][idx];
            n_idx = (y-1) * Chunk::DEPTH + z;
            const uint32_t prevY = (y-1 > 0) ? Occlusion[3][n_idx] : 0;
            const uint32_t visibleYm = current & ~prevY;

            //-------------------------
            // Meshing
            for (int x = 0; x < Chunk::WIDTH; x++) {
                auto block = chunk.get_state(x, y, z);
                if (!block) continue;
                if (!block->is_solid) continue;
                
                if (!block->model) continue;
                for (const auto &elem : block->model->elements) {
                    const glm::vec3 w = {worldOffsetX + x, static_cast<float>(y), worldOffsetZ + z};

                    const uint32_t mask = (static_cast<uint32_t>(1) << x);
                    if (shouldRender(elem.south, visibleZp & mask)) MeshZFace(vertices, w, elem, elem.south.value(),  1);
                    if (shouldRender(elem.north, visibleZm & mask)) MeshZFace(vertices, w, elem, elem.north.value(), -1);
                    if (shouldRender(elem.up, visibleYp & mask)) MeshYFace(vertices, w, elem, elem.up.value(),  1);
                    if (shouldRender(elem.down, visibleYm & mask)) MeshYFace(vertices, w, elem, elem.down.value(), -1);
                    //-------------------------
                    // X+
                    if (x + 1 < Chunk::WIDTH) {
                        if (shouldRender(elem.east, !(chunk.get_state(x+1, y, z)->model->occlusionMask & XN)))
                            MeshXFace(vertices, w, elem, elem.east.value(), 1);
                    } else if (shouldRender(elem.east, cxp && !(cxp->get_state(0, y, z)->model->occlusionMask & XN))) {
                        MeshXFace(vertices, w, elem, elem.east.value(), 1);
                    }
                    //-------------------------
                    // X-
                    if (x - 1 >= 0) {
                        if (shouldRender(elem.west, !(chunk.get_state(x-1, y, z)->model->occlusionMask & XP)))
                            MeshXFace(vertices, w, elem, elem.west.value(), -1);
                    } else if (shouldRender(elem.west, cxn && !(cxn->get_state(Chunk::WIDTH - 1, y, z)->model->occlusionMask & XP))) {
                        MeshXFace(vertices, w, elem, elem.west.value(), -1);
                    }
                }
            }
        }
    }
}

bool Mesh::shouldRender(const std::optional<Face> face, const bool visible) {
    if (!face)
        return false;

    switch (face->cull) {
        case Cull::No: return true;
        case Cull::Yes: return visible;
        case Cull::Force: return false;
    }

    return false;
}

void Mesh::getUVs(std::array<glm::vec2, 4>& outUV, const Face& face) {
    constexpr float tileSize = 1.0f / 8.0f;

    const float tileU = face.texture.x * tileSize;
    const float tileV = face.texture.y * tileSize;

    const float u0 = tileU + face.uv.x * tileSize;
    const float v0 = tileV + face.uv.y * tileSize;

    const float u1 = tileU + face.uv.z * tileSize;
    const float v1 = tileV + face.uv.w * tileSize;

    outUV[0] = {u0, v1};
    outUV[1] = {u0, v0};
    outUV[2] = {u1, v1};
    outUV[3] = {u1, v0};
}

void Mesh::MeshXFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& world, const Element &elem, const Face &face, const int dir) {
    const float z1 = world.z + elem.from.z;
    const float y1 = world.y + elem.from.y;
    const float z2 = world.z + elem.to.z;
    const float y2 = world.y + elem.to.y;

    const float x = world.x + (dir > 0 ? elem.to.x : elem.from.x);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, face);

    auto push = [&](const float x_, const float y_, const float z_, const glm::vec2 &uv_) {
        Chunk::Vertex ver;
        ver.position = {x_,y_,z_};
        ver.uv[0] = static_cast<uint16_t>(uv_.x * 65535.0f);
        ver.uv[1] = static_cast<uint16_t>(uv_.y * 65535.0f);
        ver.normal = (dir > 0 ? 1 : 0);
        vertices->push_back(ver);
    };

    if (dir > 0) { // X+
        push(x,y1,z1,uv[0]);
        push(x,y2,z1,uv[1]);
        push(x,y2,z2,uv[3]);

        push(x,y1,z1,uv[0]);
        push(x,y2,z2,uv[3]);
        push(x,y1,z2,uv[2]);
    } else { // X-
        push(x,y1,z1,uv[0]);
        push(x,y1,z2,uv[2]);
        push(x,y2,z2,uv[3]);

        push(x,y1,z1,uv[0]);
        push(x,y2,z2,uv[3]);
        push(x,y2,z1,uv[1]);
    }
}

void Mesh::MeshYFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& world, const Element &elem, const Face &face, const int dir) {
    const float x1 = world.x + elem.from.x;
    const float z1 = world.z + elem.from.z;
    const float x2 = world.x + elem.to.x;
    const float z2 = world.z + elem.to.z;

    const float y = world.y + (dir > 0 ? elem.to.y : elem.from.y);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, face);

    auto push = [&](const float x, const float y, const float z, const glm::vec2 &uv_) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint16_t>(uv_.x * 65535.0f);
        ver.uv[1] = static_cast<uint16_t>(uv_.y * 65535.0f);
        ver.normal = (dir > 0 ? 3 : 2);
        vertices->push_back(ver);
    };

    if (dir > 0) { // Y+
        push(x1, y, z1, uv[0]);
        push(x1, y, z2, uv[1]);
        push(x2, y, z2, uv[3]);

        push(x1, y, z1, uv[0]);
        push(x2, y, z2, uv[3]);
        push(x2, y, z1, uv[2]);
    } else { // Y-
        push(x1, y, z1, uv[0]);
        push(x2, y, z1, uv[2]);
        push(x2, y, z2, uv[3]);

        push(x1, y, z1, uv[0]);
        push(x2, y, z2, uv[3]);
        push(x1, y, z2, uv[1]);
    }
}

void Mesh::MeshZFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& world, const Element &elem, const Face &face, const int dir) {
    constexpr float size = 1.0f;
    const float x1 = world.x + elem.from.x;
    const float y1 = world.y + elem.from.y;
    const float x2 = world.x + elem.to.x;
    const float y2 = world.y + elem.to.y;

    const float z = world.z + (dir > 0 ? elem.to.z : elem.from.z);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, face);

    auto push = [&](const float x, const float y, const float z, const glm::vec2 &uv_) {
        Chunk::Vertex ver;
        ver.position = {x,y,z};
        ver.uv[0] = static_cast<uint16_t>(uv_.x * 65535.0f);
        ver.uv[1] = static_cast<uint16_t>(uv_.y * 65535.0f);
        ver.normal = (dir > 0 ? 5 : 4);
        vertices->push_back(ver);
    };

    if (dir > 0) { // Z+
        push(x1, y1, z, uv[0]);
        push(x2, y1, z, uv[2]);
        push(x2, y2, z, uv[3]);

        push(x1, y1, z, uv[0]);
        push(x2, y2, z, uv[3]);
        push(x1, y2, z, uv[1]);
    } else { // Z-
        push(x1, y1, z, uv[0]);
        push(x1, y2, z, uv[1]);
        push(x2, y2, z, uv[3]);

        push(x1, y1, z, uv[0]);
        push(x2, y2, z, uv[3]);
        push(x2, y1, z, uv[2]);
    }
}