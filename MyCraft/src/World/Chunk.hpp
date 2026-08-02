#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>

#include "Block.hpp"

typedef unsigned int GLuint;
typedef int GLsizei;

enum class block_type : uint16_t {
    Air,
    Stone,
    Grass,
    Dirt,
    Iron,
    Wool,
    Water,
    Lamp,
    Redstone_dust,

    _count
};

extern std::array<Block *, static_cast<int>(block_type::_count)> block_cache;
void init_block_state();

class Chunk {
public:
    struct block {
        block_type id = block_type::Air;
        uint16_t state = 0; // 0 - global  1-...

        explicit block(const block_type id = block_type::Air)
            : id(id) {}
    };

    struct Vertex {
        glm::vec3 position;
        uint16_t uv[2];
        uint8_t normal;
        uint8_t overlay[3] = {255, 255, 255};
    };

    // World Stuff
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 256;
    static constexpr int DEPTH = 16;
    static constexpr int SIZE = WIDTH*HEIGHT*DEPTH;
    std::array<block, SIZE> blocks;
    std::vector<Block *> block_state;
    std::vector<int> block_state_owner;
    int chunkX, chunkZ;

    // Mesh Stuff
    bool has_terrain = false;
    bool is_edge = false;
    bool has_mesh = false;
    bool InRender = false;
    bool DirtyFlag = true;

    std::vector<Vertex> Mesh;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei indexCount = 0;

    explicit Chunk(const int X = 0, const int Y = 0) : chunkX(X), chunkZ(Y) {}

    static int index(const int x, const int y, const int z) {
        assert((x >= 0 && x < WIDTH) && "Invalid X cordinate!");
        assert((y >= 0 && y < HEIGHT) && "Invalid Y cordinate!");
        assert((z >= 0 && z < DEPTH) && "Invalid Z cordinate!");

        const int idx = x + z * WIDTH + y * WIDTH * DEPTH;
        assert((idx >= 0 && idx < SIZE) && "Index out of bounds!");

        return idx;
    }

    [[nodiscard]] const block& get(const int x, const int y, const int z) const noexcept {
        return blocks[index(x, y, z)];
    }

    [[nodiscard]]
    Block* get_state(const int x, const int y, const int z) noexcept {
        const auto& b = blocks[index(x, y, z)];

        if (b.state == 0) {
            return block_cache[static_cast<size_t>(b.id)];
        }

        assert(b.state - 1 < block_state.size());
        assert(block_state[b.state - 1] != nullptr);
        return block_state[b.state - 1];
    }

    [[nodiscard]]
    const Block* get_state(const int x, const int y, const int z) const noexcept {
        const auto& b = blocks[index(x, y, z)];

        if (b.state == 0)
            return block_cache[static_cast<size_t>(b.id)];

        return block_state[b.state - 1];
    }

    void create_state(const int x, const int y, const int z) {
        auto &b = blocks[index(x, y, z)];
        if (b.state == 0) {
            block_state.push_back(block_cache[static_cast<size_t>(b.id)]->clone());
            block_state_owner.push_back(index(x, y, z));
            b.state = block_state.size();
        }
    }

    void set(const int x, const int y, const int z, const block& block) {
        auto &b = blocks[index(x, y, z)];
        if (b.state != 0) {
            const size_t removeIdx = b.state - 1;
            delete block_state[removeIdx];

            const size_t lastIdx = block_state.size() - 1;
            if (removeIdx != lastIdx) {
                block_state[removeIdx] = block_state[lastIdx];
                block_state_owner[removeIdx] = block_state_owner[lastIdx];

                blocks[block_state_owner[removeIdx]].state = static_cast<uint16_t>(removeIdx + 1);
            }

            block_state.pop_back();
            block_state_owner.pop_back();
        }
        b = block;
    }

    void SendData();

    void RemoveData();
};