#include "Mesh.hpp"
#include <array>

#include "World.hpp"
#include "Direction.hpp"

void Mesh::GenerateMesh(Chunk& chunk, std::vector<Chunk::Vertex> *vertices) {
    const int chunkX = chunk.chunkX;
    const int chunkZ = chunk.chunkZ;

    const auto cxp = World_Map::find_chunk(chunkX + 1, chunkZ);
    const auto cxn = World_Map::find_chunk(chunkX - 1, chunkZ);
    const auto czp = World_Map::find_chunk(chunkX, chunkZ + 1);
    const auto czn = World_Map::find_chunk(chunkX, chunkZ - 1);

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

                if (m & static_cast<int>(Face_mask::East))
                    Occlusion[static_cast<int>(Direction::East)][idx] |= 1u << x;
                if (m & static_cast<int>(Face_mask::West))
                    Occlusion[static_cast<int>(Direction::West)][idx] |= 1u << x;
                if (m & static_cast<int>(Face_mask::South))
                    Occlusion[static_cast<int>(Direction::South)][idx] |= 1u << x;
                if (m & static_cast<int>(Face_mask::North))
                    Occlusion[static_cast<int>(Direction::North)][idx] |= 1u << x;
                if (m & static_cast<int>(Face_mask::Up))
                    Occlusion[static_cast<int>(Direction::Up)][idx] |= 1u << x;
                if (m & static_cast<int>(Face_mask::Down))
                    Occlusion[static_cast<int>(Direction::Down)][idx] |= 1u << x;
            }
        }
    }

    // Visible Faces
    for (int y = 0; y < Chunk::HEIGHT; y++) {
        for (int z = 0; z < Chunk::DEPTH; z++) {
            const auto idx = y * Chunk::DEPTH + z;
            //-------------------------
            // Z+
            uint32_t current = Occlusion[static_cast<int>(Direction::South)][idx];
            uint32_t next;
            if (z + 1 < Chunk::DEPTH) {
                const auto n_idx = y * Chunk::DEPTH + (z+1);
                next = Occlusion[static_cast<int>(Direction::North)][n_idx];
            } else {
                uint32_t bits = 0;
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    if (czp) {
                        if (czp->get_state(x, y, 0)->model->occlusionMask & static_cast<int>(Face_mask::North))
                            bits |= (static_cast<uint32_t>(1) << x);
                    } else
                        bits |= (static_cast<uint32_t>(1) << x);
                }
                next = bits;
            }
            const uint32_t visibleZp = current & ~next;
            //-------------------------
            // Z-
            current = Occlusion[static_cast<int>(Direction::North)][idx];
            uint32_t prev;
            if (z - 1 >= 0) {
                const auto n_idx = y * Chunk::DEPTH + z-1;
                prev = Occlusion[static_cast<int>(Direction::South)][n_idx];
            } else {
                uint32_t bits = 0;
                for (int x = 0; x < Chunk::WIDTH; x++) {
                    if (czn) {
                        if (czn->get_state(x, y, Chunk::DEPTH - 1)->model->occlusionMask & static_cast<int>(Face_mask::South))
                            bits |= (static_cast<uint32_t>(1) << x);
                    } else
                        bits |= (static_cast<uint32_t>(1) << x);
                }
                prev = bits;
            }
            const uint32_t visibleZm = current & ~prev;
            //-------------------------
            // Y+
            current = Occlusion[static_cast<int>(Direction::Up)][idx];
            auto n_idx = (y+1) * Chunk::DEPTH + z;
            const uint32_t nextY = (y+1 < Chunk::HEIGHT) ? Occlusion[static_cast<int>(Direction::Down)][n_idx] : 0;
            const uint32_t visibleYp = current & ~nextY;
            //-------------------------
            // Y-
            current = Occlusion[static_cast<int>(Direction::Down)][idx];
            n_idx = (y-1) * Chunk::DEPTH + z;
            const uint32_t prevY = (y-1 > 0) ? Occlusion[static_cast<int>(Direction::Up)][n_idx] : 0;
            const uint32_t visibleYm = current & ~prevY;

            //-------------------------
            // Meshing
            for (int x = 0; x < Chunk::WIDTH; x++) {
                const auto block = chunk.get_state(x, y, z);
                if (!block) continue;
                if (!block->is_solid) continue;
                
                if (!block->model) continue;
                for (const auto &elem : block->model->elements) {
                    const glm::ivec3 w = {x * 16, y * 16, z * 16};

                    const uint32_t mask = (static_cast<uint32_t>(1) << x);
                    if (shouldRender(elem.south, visibleZp & mask)) MeshZFace(vertices, w, elem, block,  1);
                    if (shouldRender(elem.north, visibleZm & mask)) MeshZFace(vertices, w, elem, block, -1);
                    if (shouldRender(elem.up, visibleYp & mask)) MeshYFace(vertices, w, elem, block,  1);
                    if (shouldRender(elem.down, visibleYm & mask)) MeshYFace(vertices, w, elem, block, -1);

                    //-------------------------
                    // X+
                    if (x + 1 < Chunk::WIDTH) {
                        if (shouldRender(elem.east, !(chunk.get_state(x+1, y, z)->model->occlusionMask & static_cast<int>(Face_mask::West))))
                            MeshXFace(vertices, w, elem, block, 1);
                    } else if (shouldRender(elem.east, cxp && !(cxp->get_state(0, y, z)->model->occlusionMask & static_cast<int>(Face_mask::West)))) {
                        MeshXFace(vertices, w, elem, block, 1);
                    }
                    //-------------------------
                    // X-
                    if (x - 1 >= 0) {
                        if (shouldRender(elem.west, !(chunk.get_state(x-1, y, z)->model->occlusionMask & static_cast<int>(Face_mask::East))))
                            MeshXFace(vertices, w, elem, block, -1);
                    } else if (shouldRender(elem.west, cxn && !(cxn->get_state(Chunk::WIDTH - 1, y, z)->model->occlusionMask & static_cast<int>(Face_mask::East)))) {
                        MeshXFace(vertices, w, elem, block, -1);
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

    for (uint8_t i = 0; i < face.uvRotation; ++i) {
        const auto tmp = outUV[0];
        outUV[0] = outUV[2];
        outUV[2] = outUV[3];
        outUV[3] = outUV[1];
        outUV[1] = tmp;
    }
}

void Mesh::MeshXFace(std::vector<Chunk::Vertex> *vertices, const glm::ivec3& local, const Element &elem, Block *block, const int dir) {
    const auto &face = dir > 0 ? elem.west.value() : elem.east.value();
    const int z1 = local.z + elem.from.z;
    const int y1 = local.y + elem.from.y;
    const int z2 = local.z + elem.to.z;
    const int y2 = local.y + elem.to.y;

    const int x = local.x + (dir > 0 ? elem.to.x : elem.from.x);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, face);
    const auto overlay = block->get_overlay() * 255.0f;

    auto push = [&](const int x_, const int y_, const int z_, const glm::vec2 &uv_) {
        Chunk::Vertex ver;
        ver.position[0] = x_;
        ver.position[1] = y_;
        ver.position[2] = z_;
        ver.uv[0] = static_cast<uint16_t>(uv_.x * 65535.0f);
        ver.uv[1] = static_cast<uint16_t>(uv_.y * 65535.0f);
        ver.normal = (dir > 0 ? 1 : 0);
        ver.overlay[0] = static_cast<uint8_t>(overlay.x);
        ver.overlay[1] = static_cast<uint8_t>(overlay.y);
        ver.overlay[2] = static_cast<uint8_t>(overlay.z);
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

void Mesh::MeshYFace(std::vector<Chunk::Vertex> *vertices, const glm::ivec3& local, const Element &elem, Block *block, const int dir) {
    const auto &face = dir > 0 ? elem.up.value() : elem.down.value();
    const int x1 = local.x + elem.from.x;
    const int z1 = local.z + elem.from.z;
    const int x2 = local.x + elem.to.x;
    const int z2 = local.z + elem.to.z;

    const int y = local.y + (dir > 0 ? elem.to.y : elem.from.y);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, face);
    const auto overlay = block->get_overlay() * 255.0f;

    auto push = [&](const int x_, const int y_, const int z_, const glm::vec2 &uv_) {
        Chunk::Vertex ver;
        ver.position[0] = x_;
        ver.position[1] = y_;
        ver.position[2] = z_;
        ver.uv[0] = static_cast<uint16_t>(uv_.x * 65535.0f);
        ver.uv[1] = static_cast<uint16_t>(uv_.y * 65535.0f);
        ver.normal = (dir > 0 ? 3 : 2);
        ver.overlay[0] = static_cast<uint8_t>(overlay.x);
        ver.overlay[1] = static_cast<uint8_t>(overlay.y);
        ver.overlay[2] = static_cast<uint8_t>(overlay.z);
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

void Mesh::MeshZFace(std::vector<Chunk::Vertex> *vertices, const glm::ivec3& local, const Element &elem, Block *block, const int dir) {
    const auto &face = dir > 0 ? elem.south.value() : elem.north.value();
    const int x1 = local.x + elem.from.x;
    const int y1 = local.y + elem.from.y;
    const int x2 = local.x + elem.to.x;
    const int y2 = local.y + elem.to.y;

    const int z = local.z + (dir > 0 ? elem.to.z : elem.from.z);

    std::array<glm::vec2, 4> uv = {};
    getUVs(uv, face);
    const auto overlay = block->get_overlay() * 255.0f;

    auto push = [&](const int x_, const int y_, const int z_, const glm::vec2 &uv_) {
        Chunk::Vertex ver;
        ver.position[0] = x_;
        ver.position[1] = y_;
        ver.position[2] = z_;
        ver.uv[0] = static_cast<uint16_t>(uv_.x * 65535.0f);
        ver.uv[1] = static_cast<uint16_t>(uv_.y * 65535.0f);
        ver.normal = (dir > 0 ? 5 : 4);
        ver.overlay[0] = static_cast<uint8_t>(overlay.x);
        ver.overlay[1] = static_cast<uint8_t>(overlay.y);
        ver.overlay[2] = static_cast<uint8_t>(overlay.z);
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